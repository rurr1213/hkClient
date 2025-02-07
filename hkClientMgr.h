#pragma once

#include <stdlib.h>
#include <atomic>

#include "hkClient.h"
#include "hkDevice.h"
#include "Logger.h"

class hkClientMgr : IHKDeviceMgr
{

	HKDevice hkDevice;
	SessionInfo sessionInfo;
	json jglobalConfig;

	static int numInputMsgs;
	static int numOutputMsgs;

	std::atomic<bool> _isOpenForData = false;

public:

	hkClientMgr();
	~hkClientMgr();
	void init(HKSystemInfo& rhkSystemInfo);
	void deinit();
	void process(void);

	bool addNewConnection(void);
	bool removeConnection(void);
	virtual bool onOpenForDataEvent(void);
	virtual bool onClosedForDataEvent(void);

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
