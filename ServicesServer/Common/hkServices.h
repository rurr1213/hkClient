#pragma once

//#include <memory>
#include "hkDeviceMgr.h"
#include "hkServerServiceController.h"

class HKServices;

class ShellMsgDecoder: public MsgJsonCmdPayload {
    HKServices& hkServices;
    public:
        ShellMsgDecoder(HKServices& _hkServices) : hkServices(_hkServices ) {}
        ~ShellMsgDecoder() {}
};

class HKServices : public HKDeviceMgr
{
    std::unique_ptr<ShellMsgDecoder> pShellMsgDecoder;
    public:

        HKServerServiceController hkServerServiceController;

        HKServices();
        ~HKServices();
        bool start(void);
        bool stop(void);
        bool onConnect(void);
        bool onDisconnect(void);
        bool startServices(void);
        bool stopServices(void);
        bool registerGroupService(const std::string& groupName, std::shared_ptr<HKIServerServiceBase> pservice);
        virtual bool onPublishInfo(PublishInfo& publishInfo);

        bool processCommand(PublishInfo& publishInfo, const std::string& command);
};

