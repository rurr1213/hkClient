#pragma once
#include "hkAPI.h"
#include "hkIServerService.h"

class hkShellServerService : public HKIServerService {
        bool processCommand(PublishInfo& publishInfo, const std::string& command);

    public:
        hkShellServerService(HKIAPI& api) : HKIServerService(api) {}
        ~hkShellServerService() {}
        bool init() { return true;};
        bool deinit() { return true;};
        bool onPublishInfo(PublishInfo& publishInfo) { return true;};
};