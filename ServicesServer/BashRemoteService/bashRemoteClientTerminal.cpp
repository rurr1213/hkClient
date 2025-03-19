#include "bashRemoteClientTerminal.h"

BashRemoteClientTerminal::BashRemoteClientTerminal(HKDeviceMgr& hkDeviceMgr, const std::string &prompt)
    : BashRemoteClient(hkDeviceMgr), Terminal(prompt) {}

std::string BashRemoteClientTerminal::execute(const std::string &command, bool waitForResponse) {
    return BashRemoteClient::remoteExecute(command, waitForResponse);
}

bool BashRemoteClientTerminal::start(void) {
    return BashRemoteClient::start();
}

bool BashRemoteClientTerminal::stop(void) {
    return BashRemoteClient::stop();
}

void BashRemoteClientTerminal::run(void) {
    Terminal::run();
}