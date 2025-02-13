#include <stdlib.h>

#include "hkDeviceMgr.h"
#include "hkClient.h"

HKDeviceMgr::HKDeviceMgr()
{
    pHKClient = std::make_unique<HKClient>();
}

HKDeviceMgr::~HKDeviceMgr()
{
}

template <typename Func, typename... Args>
auto callHKClientFunc(std::unique_ptr<HKClient>& client, Func func, Args&&... args) -> decltype((client.get()->*func)(std::forward<Args>(args)...)) {
    if (client) {
        return (client.get()->*func)(std::forward<Args>(args)...);
    }
    throw std::runtime_error("HKClient is not initialized");
}

bool HKDeviceMgr::init(const ClientConnectionInfo _clientConnectionInfo) {
    clientConnectionInfo = _clientConnectionInfo;

    ConnectionInfo connectionInfo;
    connectionInfo.connectionName  = _clientConnectionInfo.connectionName;
    connectionInfo.appUUID         = _clientConnectionInfo.appUUID;
    connectionInfo.appInstallUUID  = _clientConnectionInfo.appInstallUUID;
    connectionInfo.systemName      = _clientConnectionInfo.systemName;
    connectionInfo.clientIpAddress = _clientConnectionInfo.clientIpAddress;
    connectionInfo.userName        = _clientConnectionInfo.userName;
    connectionInfo.userUUID        = _clientConnectionInfo.userUUID;
    connectionInfo.displayName     = _clientConnectionInfo.displayName;

    pHKClient->setConnectionInfo(connectionInfo);
    return callHKClientFunc(pHKClient, &HKClient::init, HYPERCUBE_SERVER_NAME_PRIMARY, true);
}

bool HKDeviceMgr::deinit(void) {
    return callHKClientFunc(pHKClient, &HKClient::deinit);
}

bool HKDeviceMgr::subscribe(std::string _groupName) {
    return callHKClientFunc(pHKClient, &HKClient::subscribe, _groupName);
}

bool HKDeviceMgr::unsubscribe(std::string _groupName) {
    return callHKClientFunc(pHKClient, &HKClient::unsubscribe, _groupName);
}

bool HKDeviceMgr::createGroup(const ClientGroupInfo clientGroupInfo)
{
    GroupInfo groupInfo;
    groupInfo.groupName = clientGroupInfo.groupName;
    return callHKClientFunc(pHKClient, &HKClient::createGroup, groupInfo);
}

bool HKDeviceMgr::sendEcho(std::string data) {
    data = clientConnectionInfo.displayName + " " + data;
    return callHKClientFunc(pHKClient, &HKClient::sendEcho, data);
}

bool HKDeviceMgr::isConnected(void) {
    return callHKClientFunc(pHKClient, &HKClient::isConnected);
}

bool HKDeviceMgr::hasReceivedData(void) {
    return callHKClientFunc(pHKClient, &HKClient::hasReceivedAPacket);
}

bool HKDeviceMgr::getPacket(Packet& packet) {
    return callHKClientFunc(pHKClient, &HKClient::getPacket, packet);
}

bool HKDeviceMgr::sendCmdMsg(std::string command) {
    MsgCmd cmdMsg(command);
    return callHKClientFunc(pHKClient, &HKClient::sendMsg, cmdMsg);
}

bool HKDeviceMgr::remotePing(void) {
    return callHKClientFunc(pHKClient, &HKClient::remotePing);
}

bool HKDeviceMgr::onOpenForDataEvent(void) {
    return true;
}

bool HKDeviceMgr::onClosedForDataEvent(void) {
    return true;
}

bool HKDeviceMgr::onReceivedDataEvent(void) {
    msgsReceived.notify();
    return true;
}


bool HKDeviceMgr::setReceiveMsgProcessor(std::unique_ptr<MsgDecoder> _pmsgDecoder) {
    pMsgDecoder = std::move(_pmsgDecoder);
    return true;
}

bool HKDeviceMgr::processReceivedMsgs(void) {

    if (!pMsgDecoder) {
        throw std::runtime_error("MsgDecoder is not initialized");
    }

    // if no messages were received, return
    if (!msgsReceived.isReady()) {
        return false;
    }

    msgsReceived.reset();

    while(callHKClientFunc(pHKClient, &HKClient::hasReceivedAPacket)) {
        PacketEx packetEx;
        packetEx.deviceId = DEVICEID::HK;
        if (getPacket(packetEx.packet)) {
            std::unique_ptr<Msg> pmsg = MsgExt::factoryMethod(packetEx);
            if (!pmsg) {
                return false;
            }
            if (MsgJson* msgJson = dynamic_cast<MsgJson*>(pmsg.get())) {
                if (!MsgExt::checkMsgJson(*msgJson)) {
                    throw std::runtime_error("MsgJson CRC failed");
                }
                std::unique_ptr<CommonInfoBase> pcommonInfoBase = pMsgDecoder->processCmdMsgJson(*msgJson);
                if (!pcommonInfoBase) throw std::runtime_error("Failed to decode Msgjson");
                PublishInfoAck* ppublishInfoAck = dynamic_cast<PublishInfoAck*>(pcommonInfoBase.get());
                if (ppublishInfoAck) {
                    publishActivity.putInfoAck(*ppublishInfoAck);
                } else {
                    throw std::runtime_error("Failed to cast to PublishInfoAck");
                }
            }
        }
    }

    return true;
}

UUIDString HKDeviceMgr::publish(std::string _groupName, std::string _data)
{
    PublishInfo publishInfo;
    publishInfo.groupName = _groupName;
    publishInfo.publishData = _data;
    if (!callHKClientFunc(pHKClient, &HKClient::publish, publishInfo)) {
        return NULL;
    }
    return publishInfo.uuid;
}

bool HKDeviceMgr::getPublishAck(UUIDString _uuid, std::string& ackData)
{
    processReceivedMsgs();
    PublishInfoAck publishInfoAck;
    if (!publishActivity.getInfoAck(_uuid, publishInfoAck)) {
        return false;
    }
    ackData = publishInfoAck.publishAckData;
    return true;
}



