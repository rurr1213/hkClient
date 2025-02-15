#pragma once

#include <stdlib.h>
#include <atomic>

#include "hkClient.h"
#include "Logger.h"
#include "hkDeviceMgrI.h"

#define BACKCHANNEL_INITIAL_SERVER_NAME "primary.hyperkube.net"

const int APPID = 8;	// used in DeviceAppKey

class SessionInfo {
public:
	int deviceAppKey;
	int sessionKey;
	void setupSession(Msg& rmsg, int seqNumber) {
		rmsg.deviceAppKey = deviceAppKey;
		rmsg.sessionKey = sessionKey;
		rmsg.seqNumber = seqNumber;
	}
};

class Stream {
	int id;
	enum STATE { CREATED, INITIALIZED, OPEN, CLOSED } chanState;
public:
	Stream() : id(0) {
		chanState = STATE::CREATED;
	};
	void setOpened(void) {	chanState = OPEN; }
	void setClosed(void) {	chanState = CLOSED;};
	bool isOpened(void) {return (chanState == OPEN);};
};

struct SPRODUCT_INFO {
	std::string platformName = "Win10";
	std::string manufacturerName = "R3R Corp";
	std::string productName = "Iris";
	std::string productId = "IRIS3840";
	double productVersion = 0.01;
	std::string productDataDir;
	std::string productRoamingDataDir;		// use to store information that Vortex can access
	std::string productInstallDir;
	std::string productExePath;
	std::string buildVersion;
	std::string buildGitTag;
	std::string buildGitShah5;
	std::string buildGitBranch;
};

class HKSystemInfo {

public:
	int deviceAppKey = 0;
	std::string connectionName = "noConnection";
	std::string appUUID = "noUUID";
	std::string appInstallUUID = "noUUID";
	std::string systemName = "noSystem";
	std::string userName = "noUser";
	std::string userUUID = "noUUID";
	std::string clientIpAddress = "noIP";

	std::string groupName = "noGroup";
};


class HKDevice : public HKClient
{
	Stream stream;
	SessionInfo& rreferenceInfo;
	IHKDeviceMgr* phkMgr;
	int numReadMessages = 0;
	int numSendMessages = 0;

public:
	HKDevice(IHKDeviceMgr* _phkMgr, SessionInfo& _rreferenceInfo);
	~HKDevice();
	bool init(std::string serverName = "", bool reInit = false);
	bool deinit();

	virtual bool onOpenForDataEvent(void);
	virtual bool onClosedForDataEvent(void);
	bool onReceivedData(void);

//	void onGlobalConfigUpdate(Config::Status& configStatus);

	bool recvFromDevice(void);
    bool getPacket(Packet& packet);
//	bool sendMsg(std::unique_ptr<Msg>& pmsg);
	bool sendMsg(Msg& rmsg, bool forceSend = false);
};

