#include "hkAPI.h"
#include "hkIServerService.h"
#include "Logger.h"

bool HKIServerService::setupGroup(void) {
    ClientGroupInfo clientGroupInfo;
    clientGroupInfo.groupName = registeredGroupName;

    bool stat = hkAPI.createGroup(clientGroupInfo);
    if (!stat) {
        LOG_ERROR("HKIServerService::setupGroup()", "Failed to create group " + registeredGroupName, 0);
        return false;
    }

    // subcribe to the bashService group
    stat = hkAPI.subscribe(registeredGroupName);
    if (!stat) {
        LOG_ERROR("HKIServerService::setupGroup()", "Failed to subscribe to group " + registeredGroupName, 0);
        return false;
    }

    return true;
}

bool HKIServerService::unSetupGroup(void) {
    // subcribe to the bashService group
    bool stat = hkAPI.unsubscribe(registeredGroupName);
    if (!stat) {
        LOG_ERROR("HKIServerService::unSetupGroup()", "Failed to subscribe to group " + registeredGroupName, 0);
        return false;
    }

    ClientGroupInfo clientGroupInfo;
    clientGroupInfo.groupName = registeredGroupName;

    stat = hkAPI.destroyGroup(clientGroupInfo);
    if (!stat) {
        LOG_ERROR("HKIServerService::unSetupGroup()", "Failed to create group " + registeredGroupName, 0);
        return false;
    }
    
    return true;
}