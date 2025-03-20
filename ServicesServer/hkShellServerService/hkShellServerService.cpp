#include "hkDeviceMgr.h"
#include "processManager.h"
#include "hkShellServerService.h"
#include "Logger.h"
#include <sstream>

HKShellServerService::HKShellServerService(HKIAPI& hkIAPI) : HKIServerServiceBase(hkIAPI)
{
}

bool HKShellServerService::init(void)
{
    return HKIServerServiceBase::init();
}

bool HKShellServerService::deinit(void)
{
    return HKIServerServiceBase::deinit();
}

bool HKShellServerService::onPublishInfo(PublishInfo& publishInfo)
{
    std::string command = publishInfo.publishData;
    std::string response = "@Shell";
    bool waitForResponse = publishInfo.ack;

    if (processCommand(publishInfo, command)) {
        return true;
    }

    return true;
}

bool HKShellServerService::processCommand(PublishInfo& publishInfo, const std::string& command)
{
    // Parse the command into words
    std::istringstream iss(command);
    std::vector<std::string> words;
    std::string word;
    while (iss >> word) {
        words.push_back(word);
    }

    if (words.size() == 0) {
        return false;
    }

    std::string domain = words[0];
    std::string action = words.size() > 1 ? words[1] : "";
    std::string parameter1 = words.size() > 2 ? words[2] : "";
    std::string parameter2 = words.size() > 3 ? words[3] : "";

    if (domain == "group") {
        if (action == "create") {
            ClientGroupInfo clientGroupInfo;
            clientGroupInfo.groupName = parameter1;
            hkAPI.createGroup(clientGroupInfo);
            hkAPI.publishAck(publishInfo, "created group: " + parameter1);
            return true;
        }
        if (action == "destroy") {
            ClientGroupInfo clientGroupInfo;
            clientGroupInfo.groupName = parameter1;
            hkAPI.destroyGroup(clientGroupInfo);
            hkAPI.publishAck(publishInfo, "destroyed group: " + parameter1);
            return true;
        }
        if (action == "subscribe") {
            hkAPI.subscribe(parameter1);
            hkAPI.publishAck(publishInfo, "subscribed to group: " + parameter1);
            return true;
        }
        if (action == "unsubscribe") {
            hkAPI.unsubscribe(parameter1);
            hkAPI.publishAck(publishInfo, "unsubscribed from group: " + parameter1);
            return true;
        }
    }
    return false;
}
