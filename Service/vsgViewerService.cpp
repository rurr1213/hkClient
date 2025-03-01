#include "hkDeviceMgr.h"
#include "processManager.h"
#include "vsgViewerService.h"


VsgViewerService::VsgViewerService(HKDeviceMgr& _hkDeviceMgr) : hkDeviceMgr(_hkDeviceMgr),
    vsgViewerProcessManager(VSGVIEWERAPPPATH, VSGVIEWERARGS)
{
}

bool VsgViewerService::runVsgViewer(void)
{
    vsgViewerProcessManager.killByName("vsgviewer");
    vsgViewerProcessManager.kill();
    bool runStatus = vsgViewerProcessManager.run();
    return runStatus;
}

bool VsgViewerService::send(const std::string& message) {
    return vsgViewerProcessManager.send(message);
}

bool VsgViewerService::receive(std::string& message) {
    return vsgViewerProcessManager.receive(message);
}

bool VsgViewerService::process(void)
{
    std::string readMsg;
    while (vsgViewerProcessManager.receive(readMsg)) {
        std::cout << "<<" << readMsg << std::endl;
    }
    return true;
}