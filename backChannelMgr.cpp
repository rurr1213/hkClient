//
#include <list>

#include "backChannelMgr.h"

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

int BackChannelMgr::numInputMsgs{ 0 };
int BackChannelMgr::numOutputMsgs{ 0 };

BackChannelMgr::BackChannelMgr() :
	backChannelDevice{this, sessionInfo}
{
}

BackChannelMgr::~BackChannelMgr() {

}

void BackChannelMgr::init(HKSystemInfo& rhkSystemInfo)
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

void BackChannelMgr::deinit(void)
{
	backChannelDevice.deinit();
}

bool BackChannelMgr::addNewConnection(void)
{
	return true;
}

bool BackChannelMgr::removeConnection(void)
{
	return true;
}

bool BackChannelMgr::onOpenForData(void)
{
	MsgDiscoveryMulticastHello msg;
//	CommMgr::createHelloMsg(msg);
	backChannelDevice.sendMsg(msg, true);

	int deviceId = (int)DEVICEID::BACKCHANNEL;
	LOG_INFO("BackChannelMgr::onOpenForData()", "DeviceIndex", (int)deviceId);
	_isOpenForData = true;
	return true;
}

bool BackChannelMgr::onClosedForData(void)
{
	_isOpenForData = false;
	int deviceId = (int)DEVICEID::BACKCHANNEL;
	LOG_INFO("BackChannelMgr::onClosedForData()", "DeviceIndex", (int)deviceId);
	return true;
}

bool BackChannelMgr::postToDevice(std::unique_ptr<Msg>& pmsg)
{
	//assert((pmsg->localParam1 == (int)DEVICEID::BACKCHANNEL);

	bool stat = backChannelDevice.sendMsg(*pmsg);

	BackChannelMgr::numOutputMsgs++;
	LOG_STATEINT("BackChannelMgr-NumOutputMsgs", BackChannelMgr::numOutputMsgs);

	return stat;
}

void BackChannelMgr::process(void)
{
	backChannelDevice.recvFromDevice();
}
/*
void BackChannelMgr::onGlobalConfigUpdate(Config::Status& configStatus)
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
		LOG_NOTE("BackChannelMgr::onGlobalConfigUpdate()", "Failed to read json value ", 0);
		return;
	}
}

void BackChannelMgr::onSystemConfigUpdate(Config::Status& configStatus)
{
}
*/