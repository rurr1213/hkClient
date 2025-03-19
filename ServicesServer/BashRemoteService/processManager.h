#pragma once

#include <string>
#include <vector>
#include <sys/types.h>
#include <unistd.h>
#include "/home/ravi/dev/photon/vsgviewer/VsgViewerPipe.h"

class ProcessManager {
    VsgViewerPipe vsgViewerPipe;
public:
    ProcessManager(const std::string& processPath, const std::vector<std::string>& arguments);
    ~ProcessManager();

    bool run();
    bool kill();
    bool killByName(const std::string& processName);
    bool send(const std::string& message);
    bool receive(std::string& message);

private:
    std::string processPath;
    std::vector<std::string> arguments;
    pid_t processPid;
    std::vector<char*> getArguments();
};