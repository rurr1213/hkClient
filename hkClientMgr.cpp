//
#include <list>

#include "hkClientMgr.h"

// ------------------------------------------------------------

BackChannelDevice::BackChannelDevice(IBackChannelMgr* _pbackChannelMgr, SessionInfo& _rreferenceInfo) :
	pbackChannelMgr{ _pbackChannelMgr},
	rreferenceInfo{ _rreferenceInfo }
{
}

BackChannelDevice::~BackChannelDevice()
{

}

void BackChannelDevice::init(std::string serverName, bool reInit)
{
	BackChannelClient::init(serverName, reInit);
}

void BackChannelDevice::deinit()
{
	BackChannelClient::deinit();
}

bool BackChannelDevice::onOpenForData(void)
{
	rreferenceInfo.sessionKey++;
	numSendMessages = -1;
	numReadMessages = 0;
	pbackChannelMgr->onOpenForData();
	stream.setOpened();
	return true;
}

bool BackChannelDevice::onClosedForData(void)
{
	if (stream.isOpened()) {
		stream.setClosed();
		pbackChannelMgr->onClosedForData();
	}
	return true;
}

bool BackChannelDevice::onReceivedData(void)
{
	HyperCubeClientCore::onReceivedData();
	return true;
}

bool BackChannelDevice::recvFromDevice(void)
{
	if (!stream.isOpened()) return false;

	bool stat = false;
	PacketEx packetEx;
	while (BackChannelClient::getPacket(packetEx.packet)) {
		packetEx.deviceId = DEVICEID::BACKCHANNEL;
		stat = true;
	}
	return stat;
}

bool BackChannelDevice::sendMsg(Msg& rmsg, bool forceSend)
{
	if (!forceSend) {
		if (!stream.isOpened()) return false;
	}

	rreferenceInfo.setupSession(rmsg, ++numSendMessages);
	rmsg.deviceAppKey = 123;
	return BackChannelClient::sendMsgOut(rmsg);
}

// ------------------------------------------------------------

int hkClientMgr::numInputMsgs{ 0 };
int hkClientMgr::numOutputMsgs{ 0 };

hkClientMgr::hkClientMgr() :
	backChannelDevice{this, sessionInfo}
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
	backChannelDevice.setConnectionInfo(connectionInfo);
	backChannelDevice.setDefaultGroupInfo(defaultGroupInfo);
	backChannelDevice.init();
}

void hkClientMgr::deinit(void)
{
	backChannelDevice.deinit();
}

bool hkClientMgr::addNewConnection(void)
{
	return true;
}

bool hkClientMgr::removeConnection(void)
{
	return true;
}

bool hkClientMgr::onOpenForData(void)
{
	MsgDiscoveryMulticastHello msg;
//	CommMgr::createHelloMsg(msg);
	backChannelDevice.sendMsg(msg, true);

	int deviceId = (int)DEVICEID::BACKCHANNEL;
	LOG_INFO("hkClientMgr::onOpenForData()", "DeviceIndex", (int)deviceId);
	_isOpenForData = true;
	return true;
}

bool hkClientMgr::onClosedForData(void)
{
	_isOpenForData = false;
	int deviceId = (int)DEVICEID::BACKCHANNEL;
	LOG_INFO("hkClientMgr::onClosedForData()", "DeviceIndex", (int)deviceId);
	return true;
}

bool hkClientMgr::postToDevice(std::unique_ptr<Msg>& pmsg)
{
	//assert((pmsg->localParam1 == (int)DEVICEID::BACKCHANNEL);

	bool stat = backChannelDevice.sendMsg(*pmsg);

	hkClientMgr::numOutputMsgs++;
	LOG_STATEINT("hkClientMgr-NumOutputMsgs", hkClientMgr::numOutputMsgs);

	return stat;
}

void hkClientMgr::process(void)
{
	backChannelDevice.recvFromDevice();
}
/*
void hkClientMgr::onGlobalConfigUpdate(Config::Status& configStatus)
{
	wGlob.m_matrix.m_systemMgr.m_config.getJsonItem(Config::GLOBAL, "backChannel", jglobalConfig);

	try {
		std::string serverIP1;
		serverIP1 = jglobalConfig["server"]["ip1"];
		if (serverIP1.size() > 0) {
			backChannelDevice.init(serverIP1, true);
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