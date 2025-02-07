//
#include <list>

#include "hkClientMgr.h"

// ------------------------------------------------------------

int hkClientMgr::numInputMsgs{ 0 };
int hkClientMgr::numOutputMsgs{ 0 };

hkClientMgr::hkClientMgr() :
	hkDevice{this, sessionInfo}
{
}

hkClientMgr::~hkClientMgr() {

}

void hkClientMgr::init(HKSystemInfo& rhkSystemInfo)
{
	sessionInfo.deviceAppKey = rhkSystemInfo.deviceAppKey;
	sessionInfo.sessionKey = 0;
	_isOpenForData = false;

	ConnectionInfo connectionInfo;
	connectionInfo.connectionName = rhkSystemInfo.connectionName;
	connectionInfo.appUUID = rhkSystemInfo.appUUID;
	connectionInfo.appInstallUUID = rhkSystemInfo.appInstallUUID;
	connectionInfo.systemName = rhkSystemInfo.systemName;
	connectionInfo.userName = rhkSystemInfo.userName;
	connectionInfo.userUUID = rhkSystemInfo.userUUID;
	connectionInfo.clientIpAddress = rhkSystemInfo.clientIpAddress;

	GroupInfo defaultGroupInfo;
	defaultGroupInfo.groupName = rhkSystemInfo.groupName;
	hkDevice.setConnectionInfo(connectionInfo);
	hkDevice.setDefaultGroupInfo(defaultGroupInfo);
	hkDevice.init();
}

void hkClientMgr::deinit(void)
{
	hkDevice.deinit();
}

bool hkClientMgr::addNewConnection(void)
{
	return true;
}

bool hkClientMgr::removeConnection(void)
{
	return true;
}

bool hkClientMgr::onOpenForDataEvent(void)
{
	MsgDiscoveryMulticastHello msg;
//	CommMgr::createHelloMsg(msg);
	hkDevice.sendMsg(msg, true);

	int deviceId = (int)DEVICEID::BACKCHANNEL;
	LOG_INFO("hkClientMgr::onOpenForDataEvent()", "DeviceIndex", (int)deviceId);
	_isOpenForData = true;
	return true;
}

bool hkClientMgr::onClosedForDataEvent(void)
{
	_isOpenForData = false;
	int deviceId = (int)DEVICEID::BACKCHANNEL;
	LOG_INFO("hkClientMgr::onClosedForDataEvent()", "DeviceIndex", (int)deviceId);
	return true;
}

bool hkClientMgr::postToDevice(std::unique_ptr<Msg>& pmsg)
{
	//assert((pmsg->localParam1 == (int)DEVICEID::BACKCHANNEL);

	bool stat = hkDevice.sendMsg(*pmsg);

	hkClientMgr::numOutputMsgs++;
	LOG_STATEINT("hkClientMgr-NumOutputMsgs", hkClientMgr::numOutputMsgs);

	return stat;
}

void hkClientMgr::process(void)
{
	hkDevice.recvFromDevice();
}
/*
void hkClientMgr::onGlobalConfigUpdate(Config::Status& configStatus)
{
	wGlob.m_matrix.m_systemMgr.m_config.getJsonItem(Config::GLOBAL, "backChannel", jglobalConfig);

	try {
		std::string serverIP1;
		serverIP1 = jglobalConfig["server"]["ip1"];
		if (serverIP1.size() > 0) {
			HKDevice.init(serverIP1, true);
		}
	}
	catch (std::exception e) {
		LOG_NOTE("hkClientMgr::onGlobalConfigUpdate()", "Failed to read json value ", 0);
		return;
	}
}

void hkClientMgr::onSystemConfigUpdate(Config::Status& configStatus)
{
}
*/