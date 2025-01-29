#include <stdio.h>

#include "Logger.h"
#ifdef _WIN64
#include <Winsock2.h> // before Windows.h, else Winsock 1 conflict
#else
#define Sleep(x) sleep(x/1000)
#endif
#include <errno.h>

#include "hkClient.h"
#include "Common.h"
#include "Packet.h"
#include "mserdes.h"
#include "kbhit.h"
#include "clockGetTime.h"
#include "MsgExt.h"
#include <regex>

using namespace std;

#ifdef _WIN64
#define poll WSAPoll
#else
#endif

// ------------------------------------------------------------------------------------------------

void HKClientCore::PacketQWithLock::init(void)
{
    deinit();
}

void HKClientCore::PacketQWithLock::deinit(void) {
    std::lock_guard<std::mutex> lock(qLock);
    while (std::deque<Packet::UniquePtr>::size() > 0) {
        std::unique_ptr<Packet> rpacket = std::move(std::deque<Packet::UniquePtr>::front());
        Packet* packet = rpacket.release();
        if (packet) delete packet;
        pop_front();
    }
}

void HKClientCore::PacketQWithLock::push(std::unique_ptr<Packet>& rpacket) {
    std::lock_guard<std::mutex> lock(qLock);
    push_back(std::move(rpacket));
}

bool HKClientCore::PacketQWithLock::pop(std::unique_ptr<Packet>& rpacket) {
    std::lock_guard<std::mutex> lock(qLock);
    if (empty()) return false;
    rpacket = std::move(std::deque<Packet::UniquePtr>::front());
    pop_front();
    return true;
}

bool HKClientCore::PacketQWithLock::isEmpty(void)
{
    std::lock_guard<std::mutex> lock(qLock);
    return empty();
}

// ----------------------------------------------------------------------


HKClientCore::RecvActivity::RecvActivity(IHKClientCore* pIHyperCubeClientCore, SignallingObject& _signallingObject) :
    CstdThread(this),
    pIHyperCubeClientCore{ pIHyperCubeClientCore },
    recvPacketBuilder(*this, COMMON_PACKETSIZE_MAX)
{};

bool HKClientCore::RecvActivity::init(void)
{
    eventReadyToRead.reset();
    std::lock_guard<std::mutex> lock(recvPacketBuilderLock);
    recvPacketBuilder.init();
    pinputPacket = std::make_unique<Packet>();
    CstdThread::init(true);
    return true;
}
bool HKClientCore::RecvActivity::deinit(void)
{
    eventReadyToRead.notify();
    // locking here causes a deadlock during shutdown as readPackets() cannot get lock to shutdown
    // No need to lock as its all being deinited anyway
    //   std::lock_guard<std::mutex> lock(recvPacketBuilderLock);
    recvPacketBuilder.deinit();
    Packet* packet = pinputPacket.release();
    if (packet) delete packet;
    inPacketQ.deinit();
    CstdThread::deinit(true);
    return true;
}

bool HKClientCore::RecvActivity::threadFunction(void)
{
    do {
        eventReadyToRead.wait();
        if (checkIfShouldExit()) break;
        RecvPacketBuilder::READSTATUS readStatus = readPackets();
        switch (readStatus) {
            case RecvPacketBuilder::READSTATUS::NEEDEDDATAREAD:
                pIHyperCubeClientCore->onReceivedData();
                break;
            case RecvPacketBuilder::READSTATUS::READERROR:
                LOG_WARNING("HKClientCore::RecvActivity::threadFunction()", "peer error", (int)readStatus);
            case RecvPacketBuilder::READSTATUS::PEERSHUTDOWN:
                pIHyperCubeClientCore->onDisconnect();
                eventReadyToRead.reset();
                break;
            case RecvPacketBuilder::READSTATUS::MOREDATANEEDED:
                break;
            default:
                LOG_WARNING("HKClientCore::RecvActivity::threadFunction()", "invalid state", (int)readStatus);
                break;
        }
    } while (!checkIfShouldExit());
    exiting();
    return true;
}

RecvPacketBuilder::READSTATUS HKClientCore::RecvActivity::readPackets(void)
{
    std::lock_guard<std::mutex> lock(recvPacketBuilderLock);

    RecvPacketBuilder::READSTATUS readStatus = recvPacketBuilder.readPacket(*pinputPacket);
    if (readStatus== RecvPacketBuilder::READSTATUS::NEEDEDDATAREAD) {
        if (!pIHyperCubeClientCore->isSignallingMsg(pinputPacket)) {
            inPacketQ.push(pinputPacket);
            pinputPacket = std::make_unique<Packet>();
            pIHyperCubeClientCore->onReceivedData();
        }
    }
    return readStatus;
}

int HKClientCore::RecvActivity::readData(void* pdata, int dataLen)
{
    int res = pIHyperCubeClientCore->tcpRecv((char*)pdata, dataLen);
    return res;
}

bool HKClientCore::RecvActivity::onConnect(void)
{
    eventReadyToRead.notify();
    return true;
}

bool HKClientCore::RecvActivity::onDisconnect(void)
{
    eventReadyToRead.reset();
    return true;
}


bool HKClientCore::RecvActivity::receiveIn(Packet::UniquePtr& rppacket) {
    bool stat = inPacketQ.pop(rppacket);
    return stat;
}

// ------------------------------------------------------------------

HKClientCore::SendActivity::SendActivity(IHKClientCore* _pIHyperCubeClientCore) :
    CstdThread(this),
    pIHyperCubeClientCore{ _pIHyperCubeClientCore },
    writePacketBuilder(COMMON_PACKETSIZE_MAX)
{};

HKClientCore::SendActivity::~SendActivity() {};

bool HKClientCore::SendActivity::init(void) {
    std::lock_guard<std::mutex> lock(writePacketBuilderLock);
    writePacketBuilder.init();
    eventPacketsAvailableToSend.reset();
    CstdThread::init(true);
    return true;
}

bool HKClientCore::SendActivity::deinit(void) {
    CstdThread::setShouldExit();
    eventPacketsAvailableToSend.notify();
    CstdThread::deinit(true);
    std::lock_guard<std::mutex> lock(writePacketBuilderLock);
    writePacketBuilder.deinit();
    outPacketQ.deinit();
    return true;
}


bool HKClientCore::SendActivity::threadFunction(void)
{
    do {
        eventPacketsAvailableToSend.wait();
        eventPacketsAvailableToSend.reset();
        if (checkIfShouldExit()) break;
        if (!writePackets()) {
            LOG_WARNING("HKClientCore::SendActivity::threadFunction()", "writePackets failed", 0);
        }
    } while (!checkIfShouldExit());
    exiting();
    return true;
}

bool HKClientCore::SendActivity::writePacket(void)
{
    std::lock_guard<std::mutex> lock(writePacketBuilderLock);
    Packet* packet = 0;

    // load packet builder if needed
    if (writePacketBuilder.empty()) {

        Packet::UniquePtr ppacket = 0;
        bool stat = outPacketQ.pop(ppacket);

        if (!stat) return true; // all sent, nothing to send

        packet = ppacket.get();
        writePacketBuilder.addNew(*packet);
    }

    // send whats in packet builder
    int numSent = sendDataOut(writePacketBuilder.getpData(), writePacketBuilder.getLength());

    if (numSent < 0) numSent = 0;
    totalBytesSent += numSent;
    bool sendDone = writePacketBuilder.setNumSent(numSent);

    return sendDone;
}

bool HKClientCore::SendActivity::writePackets(void)
{
    bool sendDone = false;
    do {
        sendDone = writePacket();
    } while (!outPacketQ.isEmpty());
    return sendDone;
}

bool HKClientCore::SendActivity::sendOut(Packet::UniquePtr& rppacket)
{
    outPacketQ.push(rppacket);
    eventPacketsAvailableToSend.notify();
    return true;
}

int HKClientCore::SendActivity::sendDataOut(const void* pdata, const int dataLen)
{
    return pIHyperCubeClientCore->tcpSend((char*)pdata, dataLen);
}

bool HKClientCore::SendActivity::onConnect(void)
{
    std::lock_guard<std::mutex> lock(writePacketBuilderLock);
    writePacketBuilder.init();
    outPacketQ.init();
    return true;
}

bool HKClientCore::SendActivity::onDisconnect(void)
{
    std::lock_guard<std::mutex> lock(writePacketBuilderLock);
    writePacketBuilder.deinit();
    outPacketQ.deinit();
    return true;
}


// ------------------------------------------------------------------------------------------------

HKClientCore::SignallingObject::SignallingObject(IHKClientCore* _pIHyperCubeClientCore) :
    pIHyperCubeClientCore{ _pIHyperCubeClientCore }, connectionId{0},
    CstdThread(this)
{
    std::srand((unsigned int)std::time(nullptr));
    connectionId = std::rand();
};

bool HKClientCore::SignallingObject::Server::updateDnsAddress(std::string _serverName)
{
    if (_serverName.length() > 0) {
        name = _serverName;
    }
    address = "";
    assert(pIHyperCubeClientCore != 0);
    if (pIHyperCubeClientCore->tcpDnsLookup(name, address)) return true;
    return false;
}

bool HKClientCore::SignallingObject::Server::init(IHKClientCore* _pIHyperCubeClientCore, std::string _serverName)
{
    pIHyperCubeClientCore = _pIHyperCubeClientCore;
    return updateDnsAddress(_serverName);
}

bool HKClientCore::SignallingObject::Servers::init(IHKClientCore* _pIHyperCubeClientCore, std::string _serverName)
{
    bool stat = false;
    servers[WORKING].init(_pIHyperCubeClientCore, _serverName);
    servers[SECONDARY].init(_pIHyperCubeClientCore);
    stat = servers[PRIMARY].init(_pIHyperCubeClientCore);
    return stat;
}

HKClientCore::SignallingObject::Server& HKClientCore::SignallingObject::Servers::getActiveServer(void)
{
    // This will drop down to the PRIMARY if dns addreses update fails
    do {
        if (activeServer.isValid()) return activeServer;
        if (activeServer.updateDnsAddress()) return activeServer;
        if (index < PRIMARY) index++;
        activeServer = servers[index];
    } while (index < PRIMARY);
    if (!activeServer.isValid()) activeServer.updateDnsAddress();
    return activeServer;
}

HKClientCore::SignallingObject::Server& HKClientCore::SignallingObject::Servers::getNextActiveServer(void)
{
    switch (index) {
    case WORKING:
        index = SECONDARY;
        break;
    case SECONDARY:
        index = PRIMARY;
        break;
    case PRIMARY:
        index = SECONDARY;
        break;
    default:
        index = PRIMARY;
        break;
    }
    assert(index <= PRIMARY);
    return getActiveServer();
}

void HKClientCore::SignallingObject::init(std::string _serverName)
{
    // lookup all addresses
    bool stat = false;

    servers.init(pIHyperCubeClientCore, _serverName);

    if (!isStarted()) {
        CstdThread::init(true);
        eventDisconnectedFromServer.reset();
    }
}

void HKClientCore::SignallingObject::deinit(void)
{
    if (isStarted()) {
        while (!isExited()) {
            setShouldExit();
            eventDisconnectedFromServer.notify();
        }
    }
    CstdThread::deinit(true);
}

bool HKClientCore::SignallingObject::threadFunction(void)
{
    LOG_INFO("HKClientCore::threadFunction()", "ThreadStarted", 0);
    while (!checkIfShouldExit()) {
        connectIfNotConnected();
        eventDisconnectedFromServer.waitUntil(HYPERCUBE_CONNECTIONINTERVAL_MS);
    }
    exiting();
    return true;
}

bool HKClientCore::SignallingObject::connectIfNotConnected(void)
{
    bool stat = true;
    if (!socketValid()) {
        if (justDisconnected) {
            // wait another second to give time, if just after a disconnect.
            // This is a seperate signalling thread, so ok to sleep here
            Sleep(2000);
            justDisconnected = false;
        }
        LOG_STATESTRING("HKClientCore-ServerIP", servers.getActiveServerAddress());
        stat = connect();
        if (stat) {
            pIHyperCubeClientCore->onConnect();
            setupConnection();
            connected = true;
        }
        eventDisconnectedFromServer.reset();

        // dont check for a while
        localPingAckTimer.init();
        numLocalPingAcks = 0;
    }
    // if local ping ack took too long, then shutdown connection and try again
    if (localPingAckTimer.isElapsed(HYPERCUBE_LOCALPINGACKTIME)) {
        uint64_t elapsedTime = localPingAckTimer.elapsed() / 1000000000;
        LOG_WARNING("HKClientCore::SignallingObject::connectIfNotConnected()", "local ping elapsed time expired closing Data and socket", (int)elapsedTime);
        pIHyperCubeClientCore->onDisconnect();
    }

    // do a periodic ping to server. This will start the localPingAckTimer and the ack will stop it
    localPing(true, "Connection status ping");

    return stat;
}

bool HKClientCore::SignallingObject::isSignallingMsg(std::unique_ptr<Packet>& rppacket)
{
    bool sigMsg = false;
    Msg msg;
    const Packet* ppacket = rppacket.get();
    assert(mserdes.packetToMsg(ppacket, msg));
    bool msgProcessed = false;
    switch (msg.subSys) {
    case SUBSYS_SIG:
        sigMsg = true;
        switch (msg.command) {
        case CMD_JSON:
            processSigMsgJson(ppacket);
            break;
        default:
            assert(false); // should not get here
        }
        break;
    default:
        break;
    }
    return sigMsg;
}

bool HKClientCore::SignallingObject::onConnectionInfoAck(HyperCubeCommand& hyperCubeCommand)
{
    ConnectionInfoAck connectionInfoAck;
    connectionInfoAck.from_json(hyperCubeCommand.getJsonData());
    std::string jsonDataString = connectionInfoAck.to_json().dump();
    if (hyperCubeCommand.status) {
        LOG_INFO("HKClientCore::SignallingObject::processSigMsgJson()", "onConnectionInfoAck, status:success", 0);
    }
    else {
        LOG_WARNING("HKClientCore::SignallingObject::processSigMsgJson()", "onConnectionInfoAckstatus:Failed - Duplicate name? " + jsonDataString, 0);
    }
    return true;
}

bool HKClientCore::SignallingObject::onCreateGroupAck(HyperCubeCommand& hyperCubeCommand)
{
    GroupInfo groupInfo;
    groupInfo.from_json(hyperCubeCommand.getJsonData());
    std::string jsonDataString = groupInfo.to_json().dump();
    if (hyperCubeCommand.status) {
        LOG_INFO("HKClientCore::SignallingObject::onCreateGroupAck()", "createGroupAck, status:success", 0);
    }
    else {
        LOG_NOTE("HKClientCore::SignallingObject::onCreateGroupAck()", "createGroupAck status:Failed - Duplicate name? " + jsonDataString, 0);
    }
    return true;
}

bool HKClientCore::SignallingObject::onRemotePing(HyperCubeCommand& hyperCubeCommand)
{
    std::string pingData = hyperCubeCommand.getJsonData().dump();
    if (hyperCubeCommand.ack) {
        LOG_INFO("HKClientCore::SignallingObject::processSigMsgJson()", "onRemotePing ack", 0);
    } else {
        LOG_INFO("HKClientCore::SignallingObject::processSigMsgJson()", "onRemotePing command", 0);
        remotePing(true, pingData);
    }
    return true;
}

bool HKClientCore::SignallingObject::onLocalPing(HyperCubeCommand& hyperCubeCommand)
{
    localPingAckTimer.stop();
    LOG_STATEINT("HKClientCore-pings", ++numLocalPingAcks);
    // std::string pingData = hyperCubeCommand.getJsonData().dump();
    // LOG_INFO("HKClientCore::SignallingObject::processSigMsgJson()", "onLocalPing ack", 0);
    return true;
}

bool HKClientCore::SignallingObject::onEchoData(HyperCubeCommand& hyperCubeCommand)
{
    std::string data = hyperCubeCommand.getJsonData().dump();
    bool status = echoData(data);
    std::string jsonDataString = hyperCubeCommand.getJsonData().dump();
    if (status) {
        LOG_INFO("HKClientCore::SignallingObject::processSigMsgJson()", "onEchoData, success", 0);
    }
    else {
        LOG_WARNING("HKClientCore::SignallingObject::processSigMsgJson()", "onEchoDat Failed" + jsonDataString, 0);
    }
    return true;
}

bool HKClientCore::SignallingObject::processSigMsgJson(const Packet* ppacket)
{
    MsgJson msgJson;
    json jsonData;
    assert(mserdes.packetToMsgJson(ppacket, msgJson, jsonData));
    bool msgProcessed = false;

    try {
        std::string logLineData = msgJson.jsonData;
        //        LOG_INFO("HKClientCore::SignallingObject::processSigMsgJson()", "received " + line, 0);
        HyperCubeCommand hyperCubeCommand(HYPERCUBECOMMANDS::NONE, NULL, true);
        hyperCubeCommand.from_json(jsonData);

        switch (hyperCubeCommand.command) {
            case HYPERCUBECOMMANDS::CONNECTIONINFOACK:
                msgProcessed = onConnectionInfoAck(hyperCubeCommand);
                break;
            case HYPERCUBECOMMANDS::CREATEGROUPACK:
                msgProcessed = onCreateGroupAck(hyperCubeCommand);
                break;
            case HYPERCUBECOMMANDS::SUBSCRIBEACK:
                LOG_INFO("HKClientCore::SignallingObject::processSigMsgJson()", "received subscribeAck" + logLineData, 0);
                msgProcessed = true;
                break;
            case HYPERCUBECOMMANDS::UNSUBSCRIBEACK:
                LOG_INFO("HKClientCore::SignallingObject::processSigMsgJson()", "received unsubscribeAck" + logLineData, 0);
                msgProcessed = true;
                break;
            case HYPERCUBECOMMANDS::SUBSCRIBER:
                LOG_INFO("HKClientCore::SignallingObject::processSigMsgJson()", "received subscriber" + logLineData, 0);
                onOpenForData();
                msgProcessed = true;
                break;
            case HYPERCUBECOMMANDS::UNSUBSCRIBER:
                LOG_INFO("HKClientCore::SignallingObject::processSigMsgJson()", "received unsubscriber" + logLineData, 0);
                onClosedForData();
                msgProcessed = true;
                break;
            case HYPERCUBECOMMANDS::CLOSEDFORDATA:
                LOG_INFO("HKClientCore::SignallingObject::processSigMsgJson()", "received onClosedForData" + logLineData, 0);
                onClosedForData();
                msgProcessed = true;
                break;
            case HYPERCUBECOMMANDS::ECHODATA:
                msgProcessed = onEchoData(hyperCubeCommand);
                break;
            case HYPERCUBECOMMANDS::REMOTEPING:
                msgProcessed = onRemotePing(hyperCubeCommand);
                break;
            case HYPERCUBECOMMANDS::LOCALPING:
                // LOG_INFO("HKClientCore::SignallingObject::processSigMsgJson()", "received LocalPing" + logLineData, 0);
                msgProcessed = onLocalPing(hyperCubeCommand);;
                break;
            default:
                break;
        }
    }
    catch (...) {
        LOG_WARNING("HKClientCore::SignallingObject::processSigMsgJson()", "Failed to decode json", 0);
    }
    return msgProcessed;
}

bool HKClientCore::SignallingObject::connect(void)
{
    bool stat = false;
    Server activeServer = servers.getActiveServer();

    stat = pIHyperCubeClientCore->tcpConnect(activeServer.name, activeServer.port);

    if (stat) {
         LOG_INFO("HKClientCore::connect()", "connected to " + servers.getActiveServerAddress(), 0);
         warnedOfFailedConnectionAttempts = 0;
         LOG_STATEINT("HKClientCore-NumSuccessfullConnectionAttempts", ++numSuccessfullConnectionAttempts);

    } else {
        LOG_STATESTRING("HKClientCore-state", "disconnected");
        LOG_STATEINT("HKClientCore-NumFailedConnectionAttempts", ++numFailedConnectionAttempts);
        if (warnedOfFailedConnectionAttempts++ < 6) {
            LOG_WARNING("HKClientCore::connect()", "connection failed to " + servers.getActiveServerAddress(), 0);
        }
        servers.getNextActiveServer();
    }
    return stat;
}

bool HKClientCore::SignallingObject::onConnect(void)
{
    LOG_STATESTRING("HKClientCore-state", "connected");
    return true;
}

bool HKClientCore::SignallingObject::onDisconnect(void)
{
    if (connected) {
        LOG_STATESTRING("HKClientCore-state", "disconnected");
        connected = false;
        justDisconnected = true;
        eventDisconnectedFromServer.notify();
    }
    return true;
}

bool HKClientCore::SignallingObject::onOpenForData(void)
{
    LOG_STATESTRING("HKClientCore-state", "openForData");
    pIHyperCubeClientCore->onOpenForData();
    return true;
}

bool HKClientCore::SignallingObject::onClosedForData(void)
{
    LOG_STATESTRING("HKClientCore-state", "closedForData");
    pIHyperCubeClientCore->onClosedForData();
    return true;
}

bool HKClientCore::SignallingObject::setupConnection(void)
{
    //sendEcho();
    sendConnectionInfo("Client");
    createDefaultGroup();
    localPing();
    LOG_INFO("HKClientCore::SignallingObject::setupConnection()", "done setup", 0);
    return true;
}

bool HKClientCore::SignallingObject::echoData(std::string echoData)
{
    LOG_INFO("HKClientCore::echoData()", "", 0);
    if (echoData.length() == 0) echoData = "echoDataData";
    StringInfo stringInfo;
    stringInfo.data = echoData;
    return sendCmdOut(HYPERCUBECOMMANDS::ECHODATA, stringInfo);
}

bool HKClientCore::SignallingObject::localPing(bool ack, std::string data)
{
    /* The following timer is critical for determing if the connection is active*/
    localPingAckTimer.start();
    StringInfo stringInfo;
    stringInfo.data = data;
    return sendCmdOut(HYPERCUBECOMMANDS::LOCALPING, stringInfo, ack);
}

bool HKClientCore::SignallingObject::remotePing(bool ack, std::string data)
{
    LOG_INFO("HKClientCore::remotePing()", "", 0);
    StringInfo stringInfo;
    stringInfo.data = data;
    return sendCmdOut(HYPERCUBECOMMANDS::REMOTEPING, stringInfo, ack);
}


bool HKClientCore::SignallingObject::publish(void)
{
    LOG_INFO("HKClientCore::publish()", "", 0);
    PublishInfo publishInfo;
    publishInfo.publishData = "publish data!";
    return sendCmdOut(HYPERCUBECOMMANDS::PUBLISHINFO, publishInfo);
}

bool HKClientCore::SignallingObject::sendConnectionInfo(std::string _connectionName)
{
    LOG_INFO("HKClientCore::sendConnectionInfo()", "", 0);
    connectionInfo.serverIpAddress = servers.getActiveServerInfo().address;
    return sendCmdOut(HYPERCUBECOMMANDS::CONNECTIONINFO, connectionInfo);
}

bool HKClientCore::SignallingObject::createGroup(std::string _groupName)
{
    LOG_INFO("HKClientCore::createGroup()", "", 0);
    GroupInfo groupInfo;
    groupInfo.groupName = _groupName;
    groupInfo.creatorConnectionInfo = connectionInfo;
    return sendCmdOut(HYPERCUBECOMMANDS::CREATEGROUP, groupInfo);
}

bool HKClientCore::SignallingObject::createGroup(const GroupInfo& _rgroupInfo)
{
    LOG_INFO("HKClientCore::createGroup()", "", 0);
    GroupInfo groupInfo = _rgroupInfo;
    groupInfo.creatorConnectionInfo = connectionInfo;
    return sendCmdOut(HYPERCUBECOMMANDS::CREATEGROUP, groupInfo);
}

bool HKClientCore::SignallingObject::createDefaultGroup(void)
{
    LOG_INFO("HKClientCore::createDefaultGroup()", "", 0);
    defaultGroupInfo.creatorConnectionInfo = connectionInfo;
    defaultGroupInfo.groupName = connectionInfo.connectionName;
    return sendCmdOut(HYPERCUBECOMMANDS::CREATEGROUP, defaultGroupInfo);
}

bool HKClientCore::SignallingObject::subscribe(std::string _groupName)
{
    LOG_INFO("HKClientCore::subscribe()", "", 0);
    SubscriberInfo subscriberInfo;
    subscriberInfo.groupName = _groupName;
    return sendCmdOut(HYPERCUBECOMMANDS::SUBSCRIBE, subscriberInfo);
}

// ------------------------------------------------------------------------------------------------

HKClientCore::HKClientCore() :
    IHKClientCore{ client },
    signallingObject{ this },
    receiveActivity{ this, signallingObject },
    sendActivity{ this }
{
};

HKClientCore::~HKClientCore() {

};

bool HKClientCore::init(std::string _serverName, bool reInit)
{
    LOG_INFO("Starting HKClientCore", "", 0);
    LOG_INFO("HKClientCore::init()", _serverName, 0);
    receiveActivity.init();
    sendActivity.init();
    signallingObject.init(_serverName);
    return true;
}

bool HKClientCore::deinit(void)
{
    signallingObject.deinit();
    client.close();
    receiveActivity.deinit();
    sendActivity.deinit();
    LOG_INFO("HKClientCore::deinit()", "", 0);
    return true;
};


/*
bool HKClientCore::sendOut(Packet::UniquePtr& ppacket)
{
    Packet* packet = ppacket.get();

    if (writePacketBuilder.empty()) {
        writePacketBuilder.addNew(*packet);
    } else return false;

    int numSent = client.send(writePacketBuilder.getpData(), writePacketBuilder.getLength());
    totalBytesSent += numSent;
    bool allSent = writePacketBuilder.setNumSent(numSent);

    if (numSent!=packet->getLength()) return false;

    return allSent;
}
*/

bool IHKClientCore::isIPAddress(const std::string& serverName) {
    // Regular expression for matching an IP address
    const std::regex ipPattern(
        R"(^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$)"
    );
    return std::regex_match(serverName, ipPattern);
}

bool IHKClientCore::tcpConnect(std::string addrString, int port)
{
    std::string ipAddr;
    if (isIPAddress(addrString))
        ipAddr = addrString;
    else
        tcpDnsLookup(addrString, ipAddr);
     return rtcpClient.connect(ipAddr, port);
}


bool HKClientCore::onConnect(void)
{
    std::string line = "connected on socket# " + std::to_string(client.getSocket());
    LOG_INFO("HKClientCore::onConnect()", line, 0);
    signallingObject.onConnect();
    receiveActivity.onConnect();
    sendActivity.onConnect();
    return true;
}

bool HKClientCore::onDisconnect(void)
{
    std::string line = "disconnect on socket# " + std::to_string(client.getSocket());
    LOG_WARNING("HKClientCore::onDisconnect()", line, 0);
    client.close();
    signallingObject.onDisconnect();
    receiveActivity.onDisconnect();
    sendActivity.onDisconnect();
    onClosedForData();
    return true;
}

bool HKClientCore::onOpenForData(void)
{
    return true;
}

bool HKClientCore::onClosedForData(void)
{
    return true;
}

bool HKClientCore::isSignallingMsg(std::unique_ptr<Packet>& rppacket)
{
    return signallingObject.isSignallingMsg(rppacket);
}

bool HKClientCore::onReceivedData(void)
{
    LOG_STATEINT("HKClientCore-numInputMsgs", ++numInputMsgs);
    return true;
}


bool HKClientCore::sendMsgOut(Msg& msg) {
    Packet::UniquePtr ppacket = 0;
    ppacket = Packet::create();
    mserdes.msgToPacket(msg, ppacket);
    bool stat = sendActivity.sendOut(ppacket);
    LOG_STATEINT("HKClientCore-numOutputMsgs", ++numOutputMsgs);
    return stat;
}

/*
bool HKClientCore::peekMsg(Msg& msg) {
    if (inPacketQ.size()<=0) return false;
    mserdes.packetToMsg(inPacketQ.front().get(), msg);
    return true;
}
*/

/*
bool HKClientCore::recvMsg(Msg& msg) {
    Packet::UniquePtr ppacket = 0;
    bool stat = receiveActivity.receiveIn(ppacket);
    if (stat) {
        mserdes.packetToMsg(ppacket.get(), msg);
    }
    return stat;
}
*/
bool HKClientCore::getPacket(Packet& packet)
{
    Packet::UniquePtr ppacket = 0;
    bool stat = receiveActivity.receiveIn(ppacket);
    if (stat) packet = std::move(*ppacket);
    return stat;
}

/*
bool HKClientCore::printRcvdMsgCmds(std::string sentString) {
    bool stat = false;
    while (!PacketQ.isEmpty()) {
        MsgCmd msgCmd("");
        mserdes.packetToMsg(inPacketQ.front().get(), msgCmd);
        std::cout << "Received cmdString: " + msgCmd.jsonData + "\n";
        if(sentString.size()>0) {
            if (msgCmd.jsonData!=sentString) {
                std::cout << "Error strings did not match " + msgCmd.jsonData +  " vs " + sentString + "\n";
            }
        }
        inPacketQ.pop_front();
    }
    return stat;
}


bool HKClientCore::processInputMsgs(std::string sentString) {
    bool stat = false;
    while (inPacketQ.size()>0) {
        MsgCmd msgCmd("");
        Packet::UniquePtr& rppacket = inPacketQ.front();
        assert(mserdes.packetToMsg(rppacket.get(), msgCmd));
        bool msgProcessed = false;
        switch(msgCmd.subSys) {
            case SUBSYS_CMD:
                switch(msgCmd.command) {
                    case CMD_JSON:
                    {
                        MsgJson msgJson;
                        json jsonData;
                        assert(mserdes.packetToMsgJson(rppacket.get(), msgJson, jsonData));
                        if (jsonData["command"]=="echoData") {
                            std::string data = jsonData["data"];
                            jsonData["command"]="echoAck";
                            MsgCmd msgCmd(jsonData.dump());
                            stat = sendMsgOut(msgCmd);
                            std::cout << "received " + mserdes.to_string(msgCmd) + jsonData.dump() + "\n";
                            msgProcessed = true;
                            break;
                        }
                        if (jsonData["command"]=="echoAck") {
                            std::string data = jsonData["data"];
                            std::cout << "received " + mserdes.to_string(msgCmd) + jsonData.dump() + "\n";
                            msgProcessed = true;
                            break;
                        }
                    }
                    break;
                }
            break;
        }
        if (!msgProcessed) cout << "ERROR! Unknown msg received " + mserdes.to_string(msgCmd) + "\n";
        inPacketQ.pop_front();
    }
    return stat;
}
*/

/*
bool HKClientCore::processConnectionEvents(void)
{
    struct pollfd pollFds[1];
    memset(pollFds, 0, sizeof(pollfd));
    int numFds = 1;

    pollFds[0].fd = getSocket();
    LOG_ASSERT(pollFds[0].fd > 0);
    pollFds[0].events = POLLOUT | POLLIN;
    pollFds[0].revents = 0;

    //int res = poll(pollFds, numFds, -1);
    /// set a timeout for this poll, so that it will exit after this timeout and loop around again.
    /// This is so that, if new sockets are added while the current poll is blocked, any data sent on the new sockets will not be
    /// received until, the current poll is unblocked and a new poll is started with the new sockets in pollFds.
    /// So, we timeout every 1 second, so that if any new sockets are added during a poll that is blocked, it will exit and
    /// come around again with the new socket added in pollFds.
    int res = poll(pollFds, numFds, 1000);
    if (res < 0) {
        LOG_WARNING("HKClientCore::processConnectionEvents()", "poll returned - 1", res);
        return false;
    }
    if ((pollFds[0].fd > 0) && (pollFds[0].revents != 0)) { // if not being ignored
    // check socket index match. It should even if a new connection was added.
        int revents = pollFds[0].revents;
        if ((revents & POLLRDNORM) || (revents & POLLRDBAND)) {
            recvPackets();
        }
        if (revents & POLLWRNORM) {
            writePackets();
        }
        if (revents & POLLPRI) {
            LOG_INFO("HKClientCore::processConnectionEvents(), poll returned POLLPRI", pollFds[0].revents);
        }
        if ((revents & POLLHUP) || (revents & POLLERR) || (revents & POLLNVAL)) {
            connectionClosed();
            LOG_INFO("HKClientCore::processConnectionEvents(), TCP connection close", (int)pollFds[0].fd);
        }
    }
    return true;
};
*/


/*
bool HKClient::doShell(void)
{
    client.init();

    if (!client.connect(serverIpAddress, SERVER_PORT)) {
        std::cout << "HKClient(): Server not available\n\r";
        return false;
    }
    bool exitNow = false;

    std::cout << "Client Interactive Mode\n\r";
    cout << "q/ESC - quit, x - exit, e - echo, s - send, r - recv, l - echo loop\n\r";
#ifdef _WIN64
    DWORD processID = GetCurrentProcessId();
    cout << "ProcessId: " << processID << endl;
#endif

    std::string dataString = "Hi There:";

    Msg inMsg;
    int msgNum = 0;

    while (!exitNow) {
        if (kbhit()) {
            char ch = getchar();
            switch (ch) {
            case 27:
            case 'q':
                exitNow = true;
                break;
            case 'p':
                publish();
                break;
            case 's':
                subscribe();
                break;
            case 'c':
                createGroup("Test groupName ");
                break;
            case 'r':
            {
                cout << "Sent SEND\n";
                string command = "SEND "; command += dataString + std::to_string(msgNum++);
                MsgCmd cmdMsg(command);
                sendMsgOut(cmdMsg);
            }
            break;
            case 'e':
                sendEcho();
                break;
            case 'x':
            {
                exitNow = true;
                MsgCmd cmdMsg("EXIT");
                sendMsgOut(cmdMsg);
                usleep(100000);
            }
            break;
            case 'l':
            {
                string command = "ECHO"; command += to_string(100) + ",";
                while (true) {
                    MsgCmd cmdMsg(command);
                    sendMsgOut(cmdMsg);
                    usleep(100000);
                }
            }
            break;
            }

        }

        recvPackets();

        printRcvdMsgCmds("");

        usleep(10000);
    }

    client.deinit();
    return true;
}


*/

// ------------------------------------------------------------

HKClient::HKClient() :
    HKClientCore{}
{
}

HKClient::~HKClient()
{
}
