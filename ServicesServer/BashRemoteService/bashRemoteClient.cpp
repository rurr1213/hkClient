#include "bashRemoteClient.h"
#include "Logger.h"
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#define Sleep(x) usleep(x*1000) // x is in milliseconds
#endif

using namespace std;

BashRemoteClient::BashRemoteClient(HKDeviceMgr& _hkDeviceMgr) : hkDeviceMgr(_hkDeviceMgr) {

}

BashRemoteClient::~BashRemoteClient()
{

}

bool BashRemoteClient::start(void)
{
    // setup group for bashService
    bool status = hkDeviceMgr.subscribe(BASHSERVICE);

    // Start bash process
    if (status)
        LOG_INFO("BashRemoteClient::start()", "subscribed to sevice", 0);
    else
        LOG_WARNING("BashRemoteClient::start()", "subscribe to sevice Failed", 0);
    return true;
}

bool BashRemoteClient::stop(void) {
    bool status = hkDeviceMgr.unsubscribe(BASHSERVICE);

    // Start bash process
    if (status)
        LOG_INFO("BashRemoteClient::start()", "unsubscribed to sevice", 0);
    else
        LOG_WARNING("BashRemoteClient::start()", "unsubscribe to sevice Failed", 0);
    return true;
}

std::string BashRemoteClient::remoteExecute(const std::string &input, bool waitForResponse)
{
    UUIDString uuid = hkDeviceMgr.publish(BASHSERVICE, input);
    if (uuid.empty()) {
        LOG_WARNING("BashRemoteClient::remoteExecute()", "publish failed", 0);
    }
    std::string info = "published command: " + input + " uuid: " + uuid;
    LOG_DBG("BashRemoteClient::remoteExecute()", info, 0);
    LOG_DBG("BashRemoteClient::remoteExecute()", "waiting for an ack", 0);
    std::string response;
    bool status = false;
    std::string groupName = BASHSERVICE;
    do {
        Sleep(100);
        status = hkDeviceMgr.waitForPublishAck(groupName, uuid, response);
        if (!status) Sleep(100);
    } while(!status);
    info = "publish Ack received:  uuid: " + uuid + " response: " + response;
    LOG_DBG("BashRemoteClient::remoteExecute()", info, 0);
    return response;
}

