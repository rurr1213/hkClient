#pragma once

#include <stdlib.h>
#include <string>
#include <memory>

#include "Packet.h"
#include "hkDeviceMgrI.h"
#include "msgDecoder.h"

class HKClient;

class HKDeviceMgr : public IHKDeviceMgr
{

    std::unique_ptr<HKClient> pHKClient;
    std::unique_ptr<MsgDecoder> pMsgDecoder;

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

        HKDeviceMgr();
        ~HKDeviceMgr();
        bool init(const ClientConnectionInfo clientConnectionInfo);
        bool deinit(void);

        bool subscribe(std::string _groupName);
        bool unsubscribe(std::string _groupName);
        bool publish(std::string _groupName, std::string data);
        bool createGroup(const ClientGroupInfo clientGroupInfo);
        bool sendEcho(std::string data);
        bool remotePing(void);
        bool isConnected(void);
        bool hasReceivedData(void);
        bool getPacket(Packet& packet);
        bool sendExit(void);
        bool sendCmdMsg(std::string Command);

        bool onOpenForDataEvent(void);
        bool onClosedForDataEvent(void);

        bool setReceiveMsgProcessor(std::unique_ptr<MsgDecoder> _pmsgDecoder);
        bool processReceivedMsgs(void);
};
