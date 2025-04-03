
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

    for (auto& service : serviceMap) {
        service.second.reset();
    }

    serviceMap.clear();
    return true;
}

bool HKServerServiceController::registerGroupService(const std::string& groupName, std::shared_ptr<HKIServerServiceBase> pservice) {
    if (pservice) {
        serviceMap[groupName] = pservice;
        pservice->registeredGroupName = groupName;
        return true;
    }
    return false;
}

bool HKServerServiceController::unregisterGroupService(const std::string& groupName) {
    if (serviceMap.find(groupName) != serviceMap.end()) {
        std::shared_ptr<HKIServerServiceBase> pservice = serviceMap[groupName];
        pservice->deinit();
        pservice = nullptr;
        serviceMap.erase(groupName);
        return true;
    }
    return false;
}

std::shared_ptr<HKIServerServiceBase> HKServerServiceController::getService(std::string groupName) {
    if (serviceMap.find(groupName) != serviceMap.end()) {
        return serviceMap[groupName];
    }
    return nullptr;
}

bool HKServerServiceController::initAllGroupServices(void) {
    for (auto& service : serviceMap) {
        service.second->init();
    }
    return true;
}

bool HKServerServiceController::deinitAllGroupServices(void) {
    for (auto& service : serviceMap) {
        service.second->deinit();
    }
    return true;
}


bool HKServerServiceController::onPublishInfo(PublishInfo& publishInfo) {
    std::string groupName = publishInfo.groupName;
    if (serviceMap.find(groupName) != serviceMap.end()) {
        return serviceMap[groupName]->onPublishInfo(publishInfo);
    }
    return false;
}


