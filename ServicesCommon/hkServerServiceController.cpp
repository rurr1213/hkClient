
#include "hkAPI.h"

/*
#include "hkShellServerService.h"
#include "vsgViewerService.h"
#include "bashRemoteService.h"
#include "hkDirService.h"
*/

bool HKServerServiceController::init() {
    // Initialization logic
    return true;
}

bool HKServerServiceController::deinit() {
    deinitAllGroupServices();

    serviceMap.deinit();
    return true;
}

bool HKServerServiceController::registerGroupService(const std::string& groupName, std::shared_ptr<HKIServerServiceBase> pservice) {
    return serviceMap.registerGroupService(groupName, pservice);
}

bool HKServerServiceController::unregisterGroupService(const std::string& groupName) {
    return serviceMap.unregisterGroupService(groupName);
}

std::shared_ptr<HKIServerServiceBase> HKServerServiceController::getService(std::string groupName) {
    return serviceMap.getService(groupName);
}

bool HKServerServiceController::initService(std::string groupName) {
    return serviceMap.initService(groupName);
}

bool HKServerServiceController::deinitService(std::string groupName) {
    return serviceMap.deinitService(groupName);
}

bool HKServerServiceController::initAllGroupServices(void) {
    return serviceMap.init();
}

bool HKServerServiceController::deinitAllGroupServices(void) {
    return serviceMap.deinit();
    return true;
}


bool HKServerServiceController::onPublishInfo(PublishInfo& publishInfo) {
    std::string groupName = publishInfo.groupName;
    std::shared_ptr<HKIServerServiceBase> phkIServerServiceBase = getService(groupName);
    if (phkIServerServiceBase) {
        return phkIServerServiceBase->onPublishInfo(publishInfo);
    }
    return false;
}


