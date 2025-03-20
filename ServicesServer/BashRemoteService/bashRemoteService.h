#pragma once
#include <string>


#include "hkDeviceMgr.h"
#include "bashProcess.h"
#include "hkIServerServiceBase.h"
#include "hkAPI.h"

class BashRemoteServer {
        BashProcess bashProcess;
    public:
        BashRemoteServer();
        ~BashRemoteServer();
        bool start(void);
        bool stop(void);
        std::string execute(const std::string &input, bool waitForResponse = true);
        std::string prompt();
};

class BashRemoteService : public HKIServerServiceBase  {
    HKIAPI& hkAPI;

    private:
        BashRemoteServer bashRemoteServer;
        std::string execute(const std::string &input, bool waitForResponse = true);

    public:

        const std::string GROUPNAME = "BashRemoteServiceGroup";

        BashRemoteService(HKIAPI& hkApi);
        ~BashRemoteService();
        bool onPublishInfo(PublishInfo& publishInfo);
};