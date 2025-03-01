#pragma once
#include <string>
#include <vector>
#include <sys/types.h>
#include "hkDeviceMgr.h"
#include "processManager.h"

#define VSGVIEWERAPPPATH "/home/ravi/dev/photon/vsgviewer/rv"
#define VSGVIEWERARGS   std::vector<std::string>{}

class VsgViewerService {

    ProcessManager vsgViewerProcessManager;
    HKDeviceMgr& hkDeviceMgr;

public:

    const std::string GROUPNAME = "VsgViewerServiceGroup";

    VsgViewerService(HKDeviceMgr& _hkDeviceMgr);
    ~VsgViewerService() {}
    bool runVsgViewer(void);
    bool process(void);
    bool send(const std::string& message);
    bool receive(std::string& message);
};