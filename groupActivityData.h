#pragma once

#include <stdlib.h>
#include <string>
#include <map>
#include <list>
#include <mutex>
#include <memory>
#include "sthread.h"

/**
 * @defgroup GroupActivityData Group Activity Data
 * @{
 */

/**
 * @brief This is a threadsafe object that can be used to add and remove group information.
 * Info can be added by a receive (producer) thread and can be retrieved by multiple
 * consumer threads. The locking is designed to be granular to allow independent
 * consumer thread access, without blocking each other.
 */
template <typename GroupName, typename Command, typename CommandInstanceInfo>
class GroupActivityData {
public:
    class CommandInstanceList : public std::list<std::shared_ptr<CommandInstanceInfo>> {
        std::mutex qLock;
        CstdConditional activityIn;
    public:
        bool add(const std::shared_ptr<CommandInstanceInfo>& pcommandInstanceInfo) {
            std::lock_guard<std::mutex> lock(qLock);
            try {
                this->push_back(pcommandInstanceInfo);
                activityIn.notify();
            } catch (...) {
                return false;
            }
            return true;
        }
        bool removeWait(std::shared_ptr<CommandInstanceInfo>& pcommandInstanceInfo, int timeoutMsSecs = 0) {
            // if timed out return false, no data
            if (timeoutMsSecs > 0)
                if (!activityIn.waitUntil(timeoutMsSecs)) return false;

            std::lock_guard<std::mutex> lock(qLock);
            if (this->empty()) return false;
            try {
                pcommandInstanceInfo = this->front();
                this->pop_front();
            } catch (...) {
                return false;
            }
            return true;
        }
    };

    class CommandsMap : public std::map<Command, std::shared_ptr<CommandInstanceList>> {
        std::mutex qLock;
    public:
        bool add(const Command& command, const std::shared_ptr<CommandInstanceInfo>& pcommandInstanceInfo) {
            std::shared_ptr<CommandInstanceList> pcommandInstanceList;
            {
                std::lock_guard<std::mutex> lock(qLock);
                pcommandInstanceList = (*this)[command];
                if (!pcommandInstanceList) {
                    pcommandInstanceList = std::make_shared<CommandInstanceList>();
                    (*this)[command] = pcommandInstanceList;
                }
            }
            return pcommandInstanceList->add(pcommandInstanceInfo);
        }
        bool removeWait(const Command& command, std::shared_ptr<CommandInstanceInfo>& pcommandInstanceInfo, int timeoutMsSecs = 0) {
            std::shared_ptr<CommandInstanceList> pcommandInstanceList;
            {
                std::lock_guard<std::mutex> lock(qLock);
                auto it = this->find(command);
                if (it == this->end()) return false;
                pcommandInstanceList = it->second;
            }
            return pcommandInstanceList->removeWait(pcommandInstanceInfo, timeoutMsSecs);
        }
    };

    /*
     * @brief Dictionary of group name to group
     */
    class GroupsMap : public std::map<GroupName, std::shared_ptr<CommandsMap>> {
        std::mutex qLock;
    public:
        bool add(const GroupName groupName, const Command& command, const std::shared_ptr<CommandInstanceInfo>& pcommandInstanceInfo) {
            std::shared_ptr<CommandsMap> pcommandsMap;
            {
                std::lock_guard<std::mutex> lock(qLock);
                pcommandsMap = (*this)[groupName];
                if (!pcommandsMap) {
                    pcommandsMap = std::make_shared<CommandsMap>();
                    (*this)[groupName] = pcommandsMap;
                }
            }
            return pcommandsMap->add(command, pcommandInstanceInfo);
        }
        bool removeWait(const GroupName groupName, const Command& command, std::shared_ptr<CommandInstanceInfo>& pcommandInstanceInfo, int timeoutMsSecs = 0) {
            std::shared_ptr<CommandsMap> pcommandsMap;
            {
                std::lock_guard<std::mutex> lock(qLock);
                auto it = this->find(groupName);
                if (it == this->end()) return false;
                pcommandsMap = it->second;
            }
            return pcommandsMap->removeWait(command, pcommandInstanceInfo, timeoutMsSecs);
        }
        bool contains(GroupName groupName) {
            std::lock_guard<std::mutex> lock(qLock);
            return (std::map<GroupName, std::shared_ptr<CommandsMap>>::find(groupName) != this->end());
        }
        bool erase(GroupName groupName) {
            std::lock_guard<std::mutex> lock(qLock);
            return std::map<GroupName, std::shared_ptr<CommandsMap>>::erase(groupName);
        }
    } groups;

    // Declare the test class as a friend
    friend class GroupActivityDataTest;
};

/** @} */ // end of GroupActivityData