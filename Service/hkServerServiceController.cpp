#include "hkIServerService.h"
#include "hkServerServiceController.h"

bool hkServerServiceController::init() {
    // Initialization logic
    return true;
}

bool hkServerServiceController::deinit() {
    // Deinitialization logic
    return true;
}

bool hkServerServiceController::registerService(const std::string& groupName, ServerServiceCode serviceCode) {
    hkIServerService* service = createService(serviceCode);
    if (service) {
        serviceMap[groupName] = service;
        return true;
    }
    return false;
}

hkIServerService* hkServerServiceController::createService(ServerServiceCode serviceCode) {
    // Factory method to create service objects based on service code
    switch (serviceCode) {
        case ServerServiceCode::SERVICE_A:
            return new hkServerServiceDefault(hkApi);
        case ServerServiceCode::SERVICE_B:
            return new hkServerServiceDefault(hkApi);
        // Add other cases as needed
        default:
            return nullptr;
    }
}

bool hkServerServiceController::onPublishInfo(PublishInfo& publishInfo) {
    std::string groupName = publishInfo.groupName;
    if (serviceMap.find(groupName) != serviceMap.end()) {
        return serviceMap[groupName]->onPublishInfo(publishInfo);
    }
    return false;
}