#pragma once

#include <map>
#include <string>
#include "hkIServerService.h"
#include "Messages.h"
#include "Logger.h"

enum class ServerServiceCode {
    SERVICE_NONE,
    SERVICE_HKSHELL,
    SERVICE_VSGVIEWER,
    SERVICE_BASH,
    SERVICE_HKDIR
};


class HKServerServiceController {
    HKIAPI& hkApi;
public:
    HKServerServiceController(HKIAPI& _hkApi) : hkApi(_hkApi) {}
    ~HKServerServiceController() {}
    bool init();
    bool deinit();
    bool registerGroupService(const std::string& groupName, ServerServiceCode serviceCode);
    bool onPublishInfo(PublishInfo& publishInfo);
    std::shared_ptr<HKIServerService> createService(ServerServiceCode serviceCode);
    std::shared_ptr<HKIServerService> getService(std::string groupName);
    bool initGroupService(std::string groupName);
    bool deinitGroupService(std::string groupName);
    bool initAllGroupServices(void);
    bool deinitAllGroupServices(void);
private:
    std::map<std::string, std::shared_ptr<HKIServerService>> serviceMap;
};
/*
template <typename ObjectClass>
bool registerAndInitGroupService(HKServerServiceController& hkServerServiceController, const std::string groupName, ServerServiceCode serviceCode)
{
    bool status = hkServerServiceController.createGroupService(groupName, serviceCode);
    if(!status) {
        LOG_ERROR("HKShell::init()", "Failed to register " + groupName, 0);
        return false;
    }
    status = hkServerServiceController.initGroupService(groupName);
    if(!status) {
        LOG_ERROR("HKShell::init()", "Failed to init " + groupName, 0);
        return false;
    }


    return true;
}


template <typename ObjectClass>
bool registerGroupService(HKServerServiceController& hkServerServiceController, const std::string groupName, ServerServiceCode serviceCode)
{
    bool status = hkServerServiceController.registerGroupService(groupName, serviceCode);
    if(!status) {
        LOG_ERROR("HKShell::init()", "Failed to register " + groupName, 0);
        return false;
    }
    return true;
}
*/
