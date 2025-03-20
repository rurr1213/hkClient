#pragma once

#include <string>

#include "hkDeviceMgrI.h"
#include "CommonCppDartCode/cpp/json/json.hpp"
#include "CommonCppDartCode/cpp/Logger.h"
#include "CommonCppDartCode/cpp/serdes.h"
#include "CommonCppDartCode/cpp/Messages.h"

typedef std::string UUIDString;

class ClientGroupInfo {
    public:
        std::string groupName = "none";
};


class HKIAPI {
public:
    virtual bool createGroup(const ClientGroupInfo clientGroupInfo) = 0;
    virtual bool destroyGroup(const ClientGroupInfo clientGroupInfo) = 0;

    virtual bool subscribe(std::string _groupName) = 0;
    virtual bool unsubscribe(std::string _groupName) = 0;

    virtual UUIDString publish(std::string _groupName, std::string data, bool ack = false) = 0;
    virtual bool publishAck(PublishInfo& publishInfo, std::string response) = 0;

};

#include "hkDeviceMgr.h"
#include "ServicesCommon/hkIServerServiceBase.h"
#include "ServicesCommon/hkServerServiceController.h"
#include "ServicesCommon/hkServices.h"

