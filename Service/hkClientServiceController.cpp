#include "hkClientServiceController.h"

bool hkClientServiceController::init() {
    // Initialization logic
    return true;
}

bool hkClientServiceController::deinit() {
    // Deinitialization logic
    return true;
}

bool hkClientServiceController::registerService(const std::string& groupName, ClientServiceCode serviceCode) {
    hkIClientService* service = createService(serviceCode);
    if (service) {
        serviceMap[groupName] = service;
        return true;
    }
    return false;
}

hkIClientService* hkClientServiceController::createService(ClientServiceCode serviceCode) {
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

bool hkClientServiceController::onPublishInfo(PublishInfo& publishInfo) {
    std::string groupName = publishInfo.groupName;
    if (serviceMap.find(groupName) != serviceMap.end()) {
        return serviceMap[groupName]->onPublishInfo(publishInfo);
    }
    return false;
}