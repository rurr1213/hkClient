#pragma once
#include <map>
#include <string>
#include "hkAPI.h"
#include "hkIClientService.h"
#include "Messages.h"

enum class ClientServiceCode {
    SERVICE_A,
    SERVICE_B,
    // Add other service codes as needed
};

class HKClientServiceController {
    HKIAPI& hkApi;
public:
    HKClientServiceController(HKIAPI& _hkApi) : hkApi(_hkApi) {}
    ~HKClientServiceController() {}
    bool init();
    bool deinit();
    bool registerService(const std::string& groupName, ClientServiceCode serviceCode);
    bool onPublishInfo(PublishInfo& publishInfo);
    HKIClientService* createService(ClientServiceCode serviceCode);

private:
    std::map<std::string, HKIClientService*> serviceMap;
};