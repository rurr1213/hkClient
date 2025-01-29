#pragma once

#include <stdlib.h>
#include <memory>

class HKClient;

class HKClientMgrExt
{
    std::unique_ptr<HKClient> pHKClient;

    public:
        HKClientMgrExt();
        ~HKClientMgrExt();
        bool init(void);
        bool deinit(void);

        bool subscribe(std::string _groupName);
        bool publish(void);
        bool createGroup(const std::string _groupName);
        bool sendEcho(std::string data);
        bool isConnected(void);
        bool sendExit(void);
        bool sendCmdMsg(std::string Command);
};
