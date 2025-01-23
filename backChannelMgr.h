#pragma once

#include <stdlib.h>
#include <atomic>

#include "backChannelClient.h"
#include "Logger.h"

#define BACKCHANNEL_INITIAL_SERVER_NAME "primary.hyperkube.net"
//#define BACKCHANNEL_INITIAL_SERVER_IP "3.141.6.1"

const int APPID = 7;	// used in DeviceAppKey

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


class IBackChannelMgr
{
public:
	virtual bool onOpenForData(void) = 0;
	virtual bool onClosedForData(void) = 0;
};

class BackChannelDevice : public BackChannelClient
{
	Stream stream;
	SessionInfo& rreferenceInfo;
	IBackChannelMgr* pbackChannelMgr;
	int numReadMessages = 0;
	int numSendMessages = 0;

public:
	BackChannelDevice(IBackChannelMgr* _pbackChannelMgr, SessionInfo& _rreferenceInfo);
	~BackChannelDevice();
	void init(std::string serverName = "", bool reInit = false);
	void deinit();

	bool onOpenForData(void);
	bool onClosedForData(void);

//	void onGlobalConfigUpdate(Config::Status& configStatus);
	bool onReceivedData(void);
	bool recvFromDevice(void);
//	bool sendMsg(std::unique_ptr<Msg>& pmsg);
	bool sendMsg(Msg& rmsg, bool forceSend = false);
};


class BackChannelMgr : IBackChannelMgr
{

	BackChannelDevice backChannelDevice;
	SessionInfo sessionInfo;
	json jglobalConfig;

	static int numInputMsgs;
	static int numOutputMsgs;

	std::atomic<bool> _isOpenForData = false;

public:

	BackChannelMgr();
	~BackChannelMgr();
	void init(HKSystemInfo& rhkSystemInfo);
	void deinit();
	void process(void);

	bool addNewConnection(void);
	bool removeConnection(void);
	virtual bool onOpenForData(void);
	virtual bool onClosedForData(void);

	bool postToDevice(std::unique_ptr<Msg>& pmsg);
//	void onGlobalConfigUpdate(Config::Status& configStatus);
//	void onSystemConfigUpdate(Config::Status& configStatus);

//	bool postToAllOpenDevices(std::unique_ptr<Msg> pmsg);
	bool postMsg(DEVICEID deviceId, std::unique_ptr<Msg> msg);
//	bool setDeviceOpen(DEVICEID deviceId);
//	bool isAnyDevices(void);

	bool botCheck(LogEvent::EVENT event) { return true; };
	bool isOpenForData(void) { return _isOpenForData;  }
private:
};
