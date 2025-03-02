#pragma once
#include <string>
#include <vector>
#include <sys/types.h>
#include "hkDeviceMgr.h"
#include "processManager.h"
#include "hkIServerService.h"

#define VSGVIEWERAPPPATH "/home/ravi/dev/photon/vsgviewer/rv"
#define VSGVIEWERARGS   std::vector<std::string>{}

class VsgViewerService : public HKIServerService  {

    ProcessManager vsgViewerProcessManager;
    bool runVsgViewer(void);
    
 public:

    const std::string GROUPNAME = "VsgViewerServiceGroup";

    VsgViewerService(HKIAPI&);
    ~VsgViewerService() {}
    bool init(void);
    bool deinit(void);
    bool onPublishInfo(PublishInfo& publishInfo);
    bool process(void);
    bool send(const std::string& message);
    bool receive(std::string& message);

};