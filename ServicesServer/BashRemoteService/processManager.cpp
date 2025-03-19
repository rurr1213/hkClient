#include "processManager.h"
#include <iostream>
#include <signal.h>
#include <sys/wait.h>
#include <cstring>
#include <cstdlib>

ProcessManager::ProcessManager(const std::string& processPath, const std::vector<std::string>& arguments)
    : processPath(processPath), arguments(arguments), processPid(0), vsgViewerPipe(VsgViewerPipe::Mode::CLIENT)
{
}

ProcessManager::~ProcessManager() {
    kill();
}

std::vector<char*> ProcessManager::getArguments() {
    std::vector<char*> args;
    args.push_back(const_cast<char*>(processPath.c_str()));
    for (const auto& arg : arguments) {
        args.push_back(const_cast<char*>(arg.c_str()));
    }
    args.push_back(nullptr);
    return args;
}

bool ProcessManager::run() {
    if (processPid != 0) {
        std::cout << "!!!! Process is already running." << std::endl;
        return false;
    }

    processPid = fork();
    if (processPid == -1) {
        std::cout << "!!!!Failed to fork process." << std::endl;
        return false;
    }

    if (processPid == 0) {
        // Child process
        std::vector<char*> args = getArguments();
        execvp(processPath.c_str(), args.data());
        // If execvp returns, there was an error
        std::cout << "!!!!!Failed to execute process." << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "PROCESS ID " << std::to_string(processPid) << std::endl;
    // Parent process
    return true;
}

bool ProcessManager::send(const std::string& message) {
    return vsgViewerPipe.write(message) > 0;
}

bool ProcessManager::receive(std::string& message) {
    return vsgViewerPipe.read(message) > 0;
}

bool ProcessManager::kill() {
    if (processPid == 0) {
        std::cout << "Process is not running." << std::endl;
        return false;
    }

    if (::kill(processPid, SIGTERM) == -1) {
        std::cout << "Failed to kill process." << std::endl;
        return false;
    }

    int status;
    waitpid(processPid, &status, 0);
    processPid = 0;
    return true;
}

bool ProcessManager::killByName(const std::string& processName) {
    std::string command = "pkill -f " + processName;
    int result = system(command.c_str());
    if (result == -1) {
        std::cout << "Failed to execute pkill command." << std::endl;
        return false;
    }
    return (result == 0);
}