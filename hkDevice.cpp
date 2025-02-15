//
#include <list>

#include "hkDevice.h"

// ------------------------------------------------------------

HKDevice::HKDevice(IHKDeviceMgr* _pbackChannelMgr, SessionInfo& _rreferenceInfo) :
	phkMgr{ _pbackChannelMgr},
	rreferenceInfo{ _rreferenceInfo }
{
}

HKDevice::~HKDevice()
{

}

bool HKDevice::init(std::string serverName, bool reInit)
{
	return HKClient::init(serverName, reInit);
}

bool HKDevice::deinit()
{
	return HKClient::deinit();
}

bool HKDevice::onOpenForDataEvent(void)
{
	rreferenceInfo.sessionKey++;
	numSendMessages = -1;
	numReadMessages = 0;
	phkMgr->onOpenForDataEvent();
	stream.setOpened();
	return true;
}

bool HKDevice::onClosedForDataEvent(void)
{
	if (stream.isOpened()) {
		stream.setClosed();
		phkMgr->onClosedForDataEvent();
	}
	return true;
}

bool HKDevice::onReceivedData(void)
{
	HKClientCore::onReceivedData();

	if (stream.isOpened()) {
		LOG_DBG("HKDevice::onReceivedData()", "received data", 0);
		phkMgr->onReceivedDataEvent();
	}
	return true;
}

bool HKDevice::recvFromDevice(void)
{
	if (!stream.isOpened()) return false;

	bool stat = false;
	PacketEx packetEx;
	while (HKClient::getPacket(packetEx.packet)) {
		packetEx.deviceId = DEVICEID::BACKCHANNEL;
		stat = true;
	}
	return stat;
}

bool HKDevice::getPacket(Packet& packet)
{
	if (!stream.isOpened()) return false;

	return HKClient::getPacket(packet);
}

bool HKDevice::sendMsg(Msg& rmsg, bool forceSend)
{
	if (!forceSend) {
		if (!stream.isOpened()) return false;
	}

	rreferenceInfo.setupSession(rmsg, ++numSendMessages);
	rmsg.deviceAppKey = 123;
	return HKClient::sendMsgOut(rmsg);
}

// ------------------------------------------------------------

