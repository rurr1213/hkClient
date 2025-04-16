#pragma once

#include <map>
#include <string>
#include <memory>
#include <mutex> // For std::mutex and std::lock_guard

#include "hkAPI.h"


class HKServiceMap {
    mutable std::mutex mtx; // Use std::mutex for thread safety
private:
    std::map<std::string, std::shared_ptr<HKIServerServiceBase>> serviceMap;
public:
    HKServiceMap() = default;
    ~HKServiceMap() {
        deinit();
    }

    bool deinit(void) {
        std::lock_guard<std::mutex> lock(mtx);
        bool stat = true;
        for (auto& service : serviceMap) {
            if (!service.second->deinit()) {
                stat = false;
            }
        }
        serviceMap.clear();
        return stat;
    }

    bool init(void) {
        std::lock_guard<std::mutex> lock(mtx);
        bool stat = true;
        for (auto& service : serviceMap) {
            if (!service.second->init()) {
                stat = false;
            }
        }
        return stat;
    }

    bool deinitAllGroupServices(void) {
        std::lock_guard<std::mutex> lock(mtx);
        for (auto& service : serviceMap) {
            if (!service.second->deinit()) {
                return false;
            }
        }
        return true;
    }

    bool registerGroupService(const std::string& groupName, std::shared_ptr<HKIServerServiceBase> pservice) {
        std::lock_guard<std::mutex> lock(mtx);
        if (pservice) {
            serviceMap[groupName] = pservice;
            pservice->registeredGroupName = groupName;
            return true;
        }
        return false;
    }

    bool unregisterGroupService(const std::string& groupName) {
        std::lock_guard<std::mutex> lock(mtx);
        if (serviceMap.find(groupName) != serviceMap.end()) {
            std::shared_ptr<HKIServerServiceBase> pservice = serviceMap[groupName];
            pservice->deinit();
            serviceMap.erase(groupName);
            pservice = nullptr;
            return true;
        }
        return false;
    }

    std::shared_ptr<HKIServerServiceBase> getService(std::string groupName) {
        std::lock_guard<std::mutex> lock(mtx);
        if (serviceMap.find(groupName) != serviceMap.end()) {
            return serviceMap[groupName];
        }
        return nullptr;
    }

    bool initService(std::string groupName) {
        std::lock_guard<std::mutex> lock(mtx);
        if (serviceMap.find(groupName) != serviceMap.end()) {
            std::shared_ptr<HKIServerServiceBase> pservice = serviceMap[groupName];
            if (pservice) {
                return pservice->init();
            } else return false;
        }
        return false;
    }
    bool deinitService(std::string groupName) {
        std::lock_guard<std::mutex> lock(mtx);
        if (serviceMap.find(groupName) != serviceMap.end()) {
            std::shared_ptr<HKIServerServiceBase> pservice = serviceMap[groupName];
            if (pservice) {
                return pservice->deinit();
            } else return false;
        }
        return false;
    }
};
