#include <stdlib.h>

#include "hkDeviceMgr.h"
#include "hkDevice.h"
#include "Logger.h"

HKDeviceMgr::HKDeviceMgr() : serviceActivity(this)
{
    SessionInfo sessionInfo;
    sessionInfo.deviceAppKey = APPID;
    pHKDevice = std::make_unique<HKDevice>(this, sessionInfo);
}

HKDeviceMgr::~HKDeviceMgr()
{
}

template <typename Func, typename... Args>
auto callHKClientFunc(std::unique_ptr<HKDevice>& device, Func func, Args&&... args) -> decltype((device.get()->*func)(std::forward<Args>(args)...)) {
    if (device) {
        return (device.get()->*func)(std::forward<Args>(args)...);
    }
    throw std::runtime_error("HKClient is not initialized");
}

bool HKDeviceMgr::init(const ClientConnectionInfo _clientConnectionInfo)
{
    serviceActivity.init();
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

    pHKDevice->setConnectionInfo(connectionInfo);
    return callHKClientFunc(pHKDevice, &HKDevice::init, HYPERCUBE_SERVER_NAME_PRIMARY, true);
}

bool HKDeviceMgr::deinit(void) {
    bool stat = callHKClientFunc(pHKDevice, &HKDevice::deinit);
    serviceActivity.deinit();
    return stat;
}

bool HKDeviceMgr::subscribe(std::string _groupName) {
    return callHKClientFunc(pHKDevice, &HKDevice::subscribe, _groupName);
}

bool HKDeviceMgr::unsubscribe(std::string _groupName) {
    return callHKClientFunc(pHKDevice, &HKDevice::unsubscribe, _groupName);
}

bool HKDeviceMgr::createGroup(const ClientGroupInfo clientGroupInfo)
{
    GroupInfo groupInfo;
    groupInfo.groupName = clientGroupInfo.groupName;
    return callHKClientFunc(pHKDevice, &HKDevice::createGroup, groupInfo);
}

bool HKDeviceMgr::sendEcho(std::string data) {
    data = clientConnectionInfo.displayName + " " + data;
    return callHKClientFunc(pHKDevice, &HKDevice::sendEcho, data);
}

bool HKDeviceMgr::isConnected(void) {
    return callHKClientFunc(pHKDevice, &HKDevice::isConnected);
}

bool HKDeviceMgr::hasReceivedData(void) {
    return callHKClientFunc(pHKDevice, &HKDevice::hasReceivedAPacket);
}

bool HKDeviceMgr::getPacket(Packet& packet) {
    return callHKClientFunc(pHKDevice, &HKDevice::getPacket, packet);
}

bool HKDeviceMgr::sendCmdMsg(std::string command) {
    MsgCmd cmdMsg(command);
    return callHKClientFunc(pHKDevice, &HKDevice::sendMsg, cmdMsg, false);
}

bool HKDeviceMgr::remotePing(void) {
    return callHKClientFunc(pHKDevice, &HKDevice::remotePing);
}

bool HKDeviceMgr::onOpenForDataEvent(void) {
    return true;
}

bool HKDeviceMgr::onClosedForDataEvent(void) {
    return true;
}

bool HKDeviceMgr::onReceivedDataEvent(void) {
    LOG_DBG("HKDeviceMgr::onReceivedDataEvent()", "received data", 0);
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

    // wait for a message
    if (!msgsReceived.waitUntil(PROCESSMSG_WAITTIMEOUT_MSECS)) return false;

    msgsReceived.reset();

    while(callHKClientFunc(pHKDevice, &HKDevice::hasReceivedAPacket)) {
        PacketEx packetEx;
        packetEx.deviceId = DEVICEID::HK;
        if (getPacket(packetEx.packet)) {
            std::unique_ptr<Msg> pmsg = MsgExt::factoryMethod(packetEx);
            if (!pmsg) {
                return false;
            }
            if (MsgJson* msgJson = dynamic_cast<MsgJson*>(pmsg.get())) {
                if (!MsgExt::checkMsgJson(*msgJson)) throw std::runtime_error("MsgJson CRC failed");

                HYPERCUBECOMMANDS command = HYPERCUBECOMMANDS::NONE;
                std::unique_ptr<CommonInfoBase> pcommonInfoBase;

                if (!pMsgDecoder->processCmdMsgJson(*msgJson, pcommonInfoBase, command)) return false;
                switch(command) {
                    case HYPERCUBECOMMANDS::PUBLISHINFO:
                    {
                        PublishInfo* ppublishInfo = dynamic_cast<PublishInfo*>(pcommonInfoBase.get());
                        if (!ppublishInfo) return false;
                        onPublishInfo(*ppublishInfo);
                    }
                    break;
                    case HYPERCUBECOMMANDS::PUBLISHINFOACK:
                    {
                        PublishInfoAck* ppublishInfoAck = dynamic_cast<PublishInfoAck*>(pcommonInfoBase.get());
                        if (!ppublishInfoAck) return false;
                        onPublishInfoAck(*ppublishInfoAck);
                    }
                    break;
                    default:
                        break;
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
    if (!callHKClientFunc(pHKDevice, &HKDevice::publish, publishInfo)) {
        return NULL;
    }
    return publishInfo.uuid;
}

bool HKDeviceMgr::publishAck(PublishInfo& publishInfo, std::string response)
{
    PublishInfoAck publishInfoAck;
    publishInfoAck.uuid = publishInfo.uuid;
    publishInfoAck.publishAckData = response;
    publishInfoAck.destinationConnectionId = publishInfo.originatingConnectionId;
    publishInfoAck.destinationUUID = publishInfo.originatorUUID;
    publishInfoAck.groupName = publishInfo.groupName;
    publishInfoAck.responderUUID = "hkShell";
    return callHKClientFunc(pHKDevice, &HKClient::publishAck, publishInfoAck);
}

bool HKDeviceMgr::waitForPublishAck(std::string groupName, UUIDString _uuid, std::string& ackData)
{
    PublishInfoAck publishInfoAck;
    std::shared_ptr<CommonInfoBase> pcommonInfoBase;
    if (!groupData.findWait(groupName, HYPERCUBECOMMANDS::PUBLISHINFOACK, _uuid, pcommonInfoBase))
        return false;

    std::shared_ptr<PublishInfoAck> ppublishInfoAck = std::dynamic_pointer_cast<PublishInfoAck>(pcommonInfoBase);
    if (!ppublishInfoAck) return false;
    ackData = ppublishInfoAck->publishAckData;
    return true;
}

bool HKDeviceMgr::onPublishInfo(PublishInfo& publishInfo)
{
    return true;
}

bool HKDeviceMgr::onPublishInfoAck(PublishInfoAck& publishInfoAck)
{
    groupData.add(publishInfoAck.groupName,
        HYPERCUBECOMMANDS::PUBLISHINFOACK,
        std::make_shared<PublishInfoAck>(publishInfoAck));
//    publishActivity.putInfoAck(publishInfoAck);
    return true;
}

/**
 * @brief This service thread loops endlessly until no longer needed.
 */
bool HKDeviceMgr::serviceActivityThread(CstdThread* pCstdThread)
{
    while(!pCstdThread->checkIfShouldExit()) {
        processReceivedMsgs();
    }
    return true;
}
