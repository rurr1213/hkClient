#pragma once

#include <stdlib.h>
#include <string>
#include <memory>

#include "Packet.h"
#include "hkDeviceMgrI.h"
#include "msgDecoder.h"
#include "sthread.h"

class HKDevice;
typedef std::string UUIDString;

class PublishActivity {
    std::map<std::string, PublishInfoAck> activityMap;
public:
    void putInfoAck(const PublishInfoAck& publishInfoAck) {
        activityMap[publishInfoAck.uuid] = publishInfoAck;
    }
    bool getInfoAck(UUIDString uuid, PublishInfoAck& publishInfoAck) {
        if (activityMap.find(uuid) == activityMap.end()) {
            return false;
        }
        publishInfoAck = activityMap[uuid];
        return true;
    }
};

class HKDeviceMgr : public IHKDeviceMgr
{

    std::unique_ptr<HKDevice> pHKDevice;
    std::unique_ptr<MsgDecoder> pMsgDecoder;
    CstdConditional msgsReceived;
    static const int RECEIVEMSG_WAITTIMEOUT_MSECS = 1000;

    PublishActivity publishActivity;

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
        UUIDString publish(std::string _groupName, std::string data);
        bool publishAck(PublishInfo& publishInfo, std::string response);
        bool getPublishAck(UUIDString _uuid, std::string& ackData);
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
        bool onReceivedDataEvent(void);

        bool setReceiveMsgProcessor(std::unique_ptr<MsgDecoder> _pmsgDecoder);
        bool processReceivedMsgs(void);

        virtual bool onPublishInfo(PublishInfo& publishInfo);
        virtual bool onPublishInfoAck(PublishInfoAck& publishInfoAck);
    };
