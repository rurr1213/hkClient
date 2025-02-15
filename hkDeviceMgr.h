#pragma once

#include <stdlib.h>
#include <string>
#include <memory>

#include "Packet.h"
#include "hkDeviceMgrI.h"
#include "msgDecoder.h"
#include "sthread.h"
#include "groupActivityData.h"

class HKDevice;
typedef std::string UUIDString;

class PublishActivity {
    std::map<std::string, PublishInfoAck> activityMap;
    std::mutex qLock;
    CstdConditional activityIn;
public:
    PublishActivity() {}
    ~PublishActivity() {}
    /**
     * @brief This can be called from a service thread
     */
    void putInfoAck(const PublishInfoAck& publishInfoAck) {
        std::lock_guard<std::mutex> lock(qLock);
        activityMap[publishInfoAck.uuid] = publishInfoAck;
        activityIn.notify();
    }
    /**
     * @brief This can be called from a UI thread
     */
    bool waitForInfoAck(UUIDString uuid, PublishInfoAck& publishInfoAck, int timeoutMsSecs) {
        if (!activityIn.waitUntil(timeoutMsSecs)) return false;
        std::lock_guard<std::mutex> lock(qLock);
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
    static const int PROCESSMSG_WAITTIMEOUT_MSECS = 1000;
    static const int PUBLISHACK_WAITTIMEOUT_MSECS = 1000;

    PublishActivity publishActivity;

    GroupActivityData<std::string, HYPERCUBECOMMANDS, CommonInfoBase> groupData;

    /**
     * @brief This class creates a service thread for serving input messages
     * after they come in and other non-time-sensitive tasks. Effectively a
     * bottom half handler.
    */
    class ServiceActivity : CstdThread {
            HKDeviceMgr* phkDeviceMgr = 0;
            virtual bool threadFunction(void) { phkDeviceMgr->serviceActivityThread(this); return true; }
        public:
            ServiceActivity(HKDeviceMgr* _phkDeviceMgr) : phkDeviceMgr(_phkDeviceMgr), CstdThread(this) {}
            bool init(void) { CstdThread::init(true); return true; }
            bool deinit(void) { CstdThread::deinit(true); return true; }
    } serviceActivity;

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
        bool waitForPublishAck(UUIDString _uuid, std::string& ackData);
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

        bool serviceActivityThread(CstdThread* pCstdThread);
    };
