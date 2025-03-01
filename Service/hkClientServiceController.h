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

class hkClientServiceController {
    hkAPI& hkApi;
public:
    hkClientServiceController(hkAPI& _hkApi) : hkApi(_hkApi) {}
    ~hkClientServiceController() {}
    bool init();
    bool deinit();
    bool registerService(const std::string& groupName, ClientServiceCode serviceCode);
    bool onPublishInfo(PublishInfo& publishInfo);
    hkIClientService* createService(ClientServiceCode serviceCode);

private:
    std::map<std::string, hkIClientService*> serviceMap;
};