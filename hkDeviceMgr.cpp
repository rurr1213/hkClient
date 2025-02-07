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

bool HKDeviceMgr::publish(std::string _groupName, std::string _data) {
    return callHKClientFunc(pHKClient, &HKClient::publish, _groupName, _data);
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

bool HKDeviceMgr::setReceiveMsgProcessor(std::unique_ptr<MsgDecoder> _pmsgDecoder) {
    pMsgDecoder = std::move(_pmsgDecoder);
    return true;
}

bool HKDeviceMgr::processReceivedMsgs(void) {
    while(callHKClientFunc(pHKClient, &HKClient::hasReceivedAPacket)) {
        PacketEx packetEx;
        packetEx.deviceId = DEVICEID::HK;
        if (getPacket(packetEx.packet)) {
            pMsgDecoder->onNewPacket(packetEx);
        }
    }
}