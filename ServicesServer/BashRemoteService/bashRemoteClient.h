#pragma once
#include <string>


#include "hkDeviceMgr.h"
#include "bashProcess.h"

class BashRemoteClient {
        HKDeviceMgr& hkDeviceMgr;
    public:
        BashRemoteClient(HKDeviceMgr& hkDeviceMgr);
        ~BashRemoteClient();
        bool start(void);
        bool stop(void);
        std::string remoteExecute(const std::string &input, bool waitForResponse = true);
};