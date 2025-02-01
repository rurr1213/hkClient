#pragma once

#include <stdlib.h>
#include <string>
#include <memory>


class HKClient;

class HKClientMgrExt
{

    std::unique_ptr<HKClient> pHKClient;

    public:
        class ClientGroupInfo {
        public:
            std::string groupName = "none";
        };

        class ClientConnectionInfo {
        public:
            std::string connectionName = "undefined";
            std::string appUUID = "noUUID";
            std::string appInstallUUID = "noUUID";
            std::string systemName = "unknown";
            std::string clientIpAddress = "notset";
            std::string userName = "notset";
            std::string userUUID = "noUUID";
            std::string displayName = "notSet";
        } clientConnectionInfo;

        HKClientMgrExt();
        ~HKClientMgrExt();
        bool init(const ClientConnectionInfo clientConnectionInfo);
        bool deinit(void);

        bool subscribe(std::string _groupName);
        bool publish(void);
        bool createGroup(const ClientGroupInfo clientGroupInfo);
        bool sendEcho(std::string data);
        bool remotePing(void);
        bool isConnected(void);
        bool sendExit(void);
        bool sendCmdMsg(std::string Command);
};
