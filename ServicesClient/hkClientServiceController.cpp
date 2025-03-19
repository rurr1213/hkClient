#include "hkClientServiceController.h"

bool HKClientServiceController::init() {
    // Initialization logic
    return true;
}

bool HKClientServiceController::deinit() {
    // Deinitialization logic
    return true;
}

bool HKClientServiceController::registerService(const std::string& groupName, ClientServiceCode serviceCode) {
    HKIClientService* service = createService(serviceCode);
    if (service) {
        serviceMap[groupName] = service;
        return true;
    }
    return false;
}

HKIClientService* HKClientServiceController::createService(ClientServiceCode serviceCode) {
    // Factory method to create service objects based on service code
    switch (serviceCode) {
        case ClientServiceCode::SERVICE_A:
            return new hkIClientServiceDefault(hkApi);
        case ClientServiceCode::SERVICE_B:
            return new hkIClientServiceDefault(hkApi);
        // Add other cases as needed
        default:
            return nullptr;
    }
}

bool HKClientServiceController::onPublishInfo(PublishInfo& publishInfo) {
    std::string groupName = publishInfo.groupName;
    if (serviceMap.find(groupName) != serviceMap.end()) {
        return serviceMap[groupName]->onPublishInfo(publishInfo);
    }
    return false;
}