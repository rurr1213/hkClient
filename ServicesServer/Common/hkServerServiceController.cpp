#include "hkIServerService.h"
#include "hkServerServiceController.h"

#include "hkShellServerService.h"
#include "vsgViewerService.h"
#include "bashRemoteService.h"
#include "hkDirService.h"

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

bool HKServerServiceController::registerGroupService(const std::string& groupName, ServerServiceCode serviceCode) {
    std::shared_ptr<HKIServerService> pservice = createService(serviceCode);
    if (pservice) {
        serviceMap[groupName] = pservice;
        pservice->registeredGroupName = groupName;
        return true;
    }
    return false;
}

std::shared_ptr<HKIServerService> HKServerServiceController::getService(std::string groupName) {
    if (serviceMap.find(groupName) != serviceMap.end()) {
        return serviceMap[groupName];
    }
    return nullptr;
}

bool HKServerServiceController::initGroupService(std::string groupName) {
    if (serviceMap.find(groupName) != serviceMap.end()) {
        return serviceMap[groupName]->init();
    }
    return false;
}

bool HKServerServiceController::deinitGroupService(std::string groupName) {
    if (serviceMap.find(groupName) != serviceMap.end()) {
        return serviceMap[groupName]->deinit();
    }
    return false;
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


std::shared_ptr<HKIServerService> HKServerServiceController::createService(ServerServiceCode serviceCode) {

    // Factory method to create service objects based on service code
    std::shared_ptr<HKIServerService> phkIServerService;
    switch (serviceCode) {
        case ServerServiceCode::SERVICE_HKSHELL:
            phkIServerService = std::make_shared<HKShellServerService>(hkApi);
            break;
        case ServerServiceCode::SERVICE_VSGVIEWER:
            phkIServerService = std::make_shared<VsgViewerService>(hkApi);
            break;
        case ServerServiceCode::SERVICE_BASH:
            phkIServerService = std::make_shared<BashRemoteService>(hkApi);
            break;
        case ServerServiceCode::SERVICE_HKDIR:
            phkIServerService = std::make_shared<HKDirService>(hkApi);
            break;
        case ServerServiceCode::SERVICE_NONE:
        default:
            break;
    }
    return phkIServerService;
}

