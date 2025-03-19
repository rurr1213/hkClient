#pragma once

#include <map>
#include <string>
#include "hkIServerService.h"
#include "Messages.h"
#include "Logger.h"


class HKServerServiceController {
    HKIAPI& hkApi;
public:
    HKServerServiceController(HKIAPI& _hkApi) : hkApi(_hkApi) {}
    ~HKServerServiceController() {}
    bool init();
    bool deinit();
    bool registerGroupService(const std::string& groupName, std::shared_ptr<HKIServerService> pservice);
    std::shared_ptr<HKIServerService> getService(std::string groupName);
    bool initAllGroupServices(void);
    bool deinitAllGroupServices(void);
    bool onPublishInfo(PublishInfo& publishInfo);
//    bool initGroupService(std::string groupName);
//    bool deinitGroupService(std::string groupName);

private:
    std::map<std::string, std::shared_ptr<HKIServerService>> serviceMap;
};
