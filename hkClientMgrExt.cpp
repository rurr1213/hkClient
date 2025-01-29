#include <stdlib.h>

#include "hkClientMgrExt.h"
#include "hkClient.h"

HKClientMgrExt::HKClientMgrExt()
{
    pHKClient = std::make_unique<HKClient>();
}

HKClientMgrExt::~HKClientMgrExt()
{
}

template <typename Func, typename... Args>
auto callHKClientFunc(std::unique_ptr<HKClient>& client, Func func, Args&&... args) -> decltype((client.get()->*func)(std::forward<Args>(args)...)) {
    if (client) {
        return (client.get()->*func)(std::forward<Args>(args)...);
    }
    throw std::runtime_error("HKClient is not initialized");
}

bool HKClientMgrExt::init(void) {
    return callHKClientFunc(pHKClient, &HKClient::init, HYPERCUBE_SERVER_NAME_PRIMARY, true);
}

bool HKClientMgrExt::deinit(void) {
    return callHKClientFunc(pHKClient, &HKClient::deinit);
}

bool HKClientMgrExt::subscribe(std::string _groupName) {
    return callHKClientFunc(pHKClient, &HKClient::subscribe, _groupName);
}

bool HKClientMgrExt::publish(void) {
    return callHKClientFunc(pHKClient, &HKClient::publish);
}

bool HKClientMgrExt::createGroup(const std::string _groupName)
{
    return callHKClientFunc(pHKClient, &HKClient::createGroup, _groupName);
}

bool HKClientMgrExt::sendEcho(std::string data) {
    return callHKClientFunc(pHKClient, &HKClient::sendEcho, data);
}

bool HKClientMgrExt::isConnected(void) {
    return callHKClientFunc(pHKClient, &HKClient::isConnected);
}

bool HKClientMgrExt::sendCmdMsg(std::string command) {
    MsgCmd cmdMsg(command);
    return callHKClientFunc(pHKClient, &HKClient::sendMsg, cmdMsg);
}