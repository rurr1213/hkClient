#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <sys/wait.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/prctl.h>

#include "kbhit.h"

#include "hkServices.h"
#include "Logger.h"

/* Helper functions */
std::string getCurrentTimeStampString(void) {
    auto now = std::chrono::system_clock::now();
     auto now_c = std::chrono::system_clock::to_time_t(now);
     std::stringstream ss;
     ss << std::put_time(std::localtime(&now_c), "%Y%m%d%H%M%S");
     return ss.str();
}

HKServices::HKServices() :
    hkServerServiceController(*this)
{
}

HKServices::~HKServices() {
}

bool HKServices::start(void) {

    pShellMsgDecoder = std::make_unique<ShellMsgDecoder>(*this);

    int connectionId = rand() % 20;
    HKDeviceMgr::ClientConnectionInfo clientConnectionInfo;

    clientConnectionInfo.connectionName = "HKServices_" + getCurrentTimeStampString();
    clientConnectionInfo.appUUID = "HKServices";
    clientConnectionInfo.appInstallUUID = "HKServices";
    clientConnectionInfo.systemName = "HKServices";
    clientConnectionInfo.userName = "HKServices";
    clientConnectionInfo.userUUID = "HKServices";
    clientConnectionInfo.displayName = clientConnectionInfo.connectionName;

    std::unique_ptr<MsgJsonCmdPayload> pMsgDecoder = std::move(pShellMsgDecoder);
    HKDeviceMgr::setReceiveMsgProcessor(std::move(pMsgDecoder));
    bool status = HKDeviceMgr::init(clientConnectionInfo);

    return status;
}

bool HKServices::stop(void) {
    pShellMsgDecoder  = nullptr;

    return HKDeviceMgr::deinit();
}

bool HKServices::registerGroupService(const std::string& groupName, std::shared_ptr<HKIServerServiceBase> pservice) {
    if (!hkServerServiceController.registerGroupService(groupName, pservice))
        return false;

    return true;
}

bool HKServices::startServices(void) {

    bool status = hkServerServiceController.init();
    if(!status) {
        LOG_ERROR("HKServices::init()", "Failed to initialize hkServerServiceController", 0);
        return false;
    }

    status = hkServerServiceController.initAllGroupServices();
    if(!status) {
        LOG_ERROR("HKServices::init()", "Failed to initialize All Group services", 0);
        return false;
    }

    return true;
}

bool HKServices::stopServices(void) {

    bool status = hkServerServiceController.deinitAllGroupServices();
    hkServerServiceController.deinit();
    return status;
}

bool HKServices::onConnect(void) {
    return startServices();
}

bool HKServices::onDisconnect(void) {
    return stopServices();
}


bool HKServices::onPublishInfo(PublishInfo& publishInfo)
{
    return hkServerServiceController.onPublishInfo(publishInfo);
}
