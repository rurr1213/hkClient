#include "hkDeviceMgr.h"
#include "hkDirService.h"
#include "Logger.h"
#include <dirent.h>
#include <sys/types.h>
#include <sstream>
#include <cstdlib> // For getenv
#include "json.hpp"

using json = nlohmann::json;

HKDirService::HKDirService(HKIAPI& api) : HKIServerServiceBase(api) {

}

HKDirService::~HKDirService() {

}

bool HKDirService::onPublishInfo(PublishInfo& publishInfo)
{
    std::string publishData = publishInfo.publishData;
    std::string response = GROUPNAME;
    bool waitForResponse = publishInfo.ack;

    std::istringstream iss(publishData);
    std::vector<std::string> words;
    std::string word;
    while (iss >> word) {
        words.push_back(word);
    }
    std::string command = words.size() > 0 ? words[0] : "";
    std::string parameter1 = words.size() > 1 ? words[1] : "";
    std::string parameter2 = words.size() > 2 ? words[2] : "";

    if (command == "getDir") {
        std::string path = words.size() > 1 ? words[1] : currentDir;
        response = getDirNodes(path);
        hkAPI.publishAck(publishInfo, response);
        return true;
    }

    return true;
}

std::string HKDirService::getDirNodes(std::string path)
{
    // Expand tilde to home directory if it is at the beginning of the path
    if (!path.empty() && path[0] == '~') {
        const char* homeDir = getenv("HOME");
        if (homeDir != nullptr) {
            path.replace(0, 1, homeDir);
        } else {
            json responseJson;
            responseJson["status"] = "Error: HOME environment variable not set.";
            return responseJson.dump();
        }
    }

    DIR* dir = opendir(path.c_str());
    if (dir == nullptr) {
        json responseJson;
        responseJson["status"] = "Error: Failed to open directory: " + path;
        return responseJson.dump();
    }

    json responseJson;
    responseJson["directory"] = path;
    responseJson["contents"] = json::array();

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        json nodeJson;
        nodeJson["name"] = entry->d_name;
        switch (entry->d_type) {
            case DT_REG: nodeJson["type"] = "File"; break;
            case DT_DIR: nodeJson["type"] = "Directory"; break;
            case DT_LNK: nodeJson["type"] = "Symlink"; break;
            default: nodeJson["type"] = "Other"; break;
        }
        responseJson["contents"].push_back(nodeJson);
    }
    closedir(dir);

    return responseJson.dump();
}