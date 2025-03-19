#pragma once

#include "terminal.h"
#include "bashRemoteClient.h"

class BashRemoteClientTerminal : protected Terminal, protected BashRemoteClient {
protected:
    std::string execute(const std::string &command, bool waitForResponse = true) override;

public:
    BashRemoteClientTerminal(HKDeviceMgr& hkDeviceMgr, const std::string &prompt = "> ");
    bool start(void);
    void run(void);
    bool stop(void);
};