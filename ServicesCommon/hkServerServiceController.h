#pragma once

#include <map>
#include <string>

#include "hkAPI.h"
#include "hkServiceMap.h"

class HKServerServiceController {
    HKIAPI& hkApi;
public:
    HKServerServiceController(HKIAPI& _hkApi) : hkApi(_hkApi) {}
    ~HKServerServiceController() {}
    bool init();
    bool deinit();
    bool registerGroupService(const std::string& groupName, std::shared_ptr<HKIServerServiceBase> pservice);
    bool unregisterGroupService(const std::string& groupName);
    std::shared_ptr<HKIServerServiceBase> getService(std::string groupName);
    bool initAllGroupServices(void);
    bool deinitAllGroupServices(void);
    bool onPublishInfo(PublishInfo& publishInfo);
    bool initService(std::string groupName);
    bool deinitService(std::string groupName);

private:
    HKServiceMap serviceMap;
};
