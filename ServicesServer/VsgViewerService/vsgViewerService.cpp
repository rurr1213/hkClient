//#include "hkDeviceMgr.h"
//#include "processManager.h"
#include "vsgViewerService.h"
#include "Logger.h"

VsgViewerService::VsgViewerService(HKIAPI& hkIAPI) : HKIServerServiceBase(hkIAPI),
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

bool VsgViewerService::onPublishInfo(PublishInfo& publishInfo)
{
    std::string command = publishInfo.publishData;
    std::string response = "vsgViewerService";
    bool waitForResponse = publishInfo.ack;

    if (command == "@rv") {
        bool runStatus = runVsgViewer();
        LOG_DBG("HKShell::onPublishInfo() ack: ", response.substr(0, 60), 0);
        LOG_INFO("HKShell::onPublishInfo() RUNNING RV ", command, 0);
        std::string response;
        if (runStatus) {
            response = "vsgViewerService@ started";
        } else {
            response = "vsgViewerService@ failed to start";
        }
        hkAPI.publishAck(publishInfo, response);
        return true;
    }

    return send(command);
}
