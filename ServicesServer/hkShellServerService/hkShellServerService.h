#pragma once
#include "hkAPI.h"
#include "hkIServerService.h"

class HKShellServerService : public HKIServerService {
        bool processCommand(PublishInfo& publishInfo, const std::string& command);

    public:
        HKShellServerService(HKIAPI&);
        ~HKShellServerService() {}
        bool init(void) override;
        bool deinit(void) override;
        bool onPublishInfo(PublishInfo& publishInfo) override;
};