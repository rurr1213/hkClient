#pragma once

#include <map>
#include <string>
#include "hkIServerService.h"
#include "Messages.h"

enum class ServerServiceCode {
    SERVICE_A,
    SERVICE_B,
    // Add other service codes as needed
};

class hkServerServiceController {
    hkAPI& hkApi;
public:
    hkServerServiceController(hkAPI& _hkApi) : hkApi(_hkApi) {}
    ~hkServerServiceController() {}
    bool init();
    bool deinit();
    bool registerService(const std::string& groupName, ServerServiceCode serviceCode);
    bool onPublishInfo(PublishInfo& publishInfo);
    hkIServerService* createService(ServerServiceCode serviceCode);

private:
    std::map<std::string, hkIServerService*> serviceMap;
};