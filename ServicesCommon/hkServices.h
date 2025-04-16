#pragma once

//#include "hkDeviceMgr.h"
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
        bool init(void);
        bool deinit(void);
        bool onConnect(void);
        bool onDisconnect(void);
        bool startServices(void);
        bool stopServices(void);
        bool registerGroupService(const std::string& groupName, std::shared_ptr<HKIServerServiceBase> pservice);
        bool unregisterGroupService(const std::string& groupName);
        virtual bool onPublishInfo(PublishInfo& publishInfo);

        bool initService(const std::string& groupName);
        bool deinitService(const std::string& groupName);

        bool processCommand(PublishInfo& publishInfo, const std::string& command);
};

