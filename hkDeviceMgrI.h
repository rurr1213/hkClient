#pragma once

class PublishInfo;
class PublishInfoAck;


class IHKDeviceMgr
{
public:
	virtual bool onOpenForDataEvent(void) = 0;
	virtual bool onClosedForDataEvent(void) = 0;
	virtual bool onReceivedDataEvent(void) = 0;
	virtual bool onPublishInfo(PublishInfo& publishInfo) = 0;
	virtual bool onPublishInfoAck(PublishInfoAck& publishInfoAck) = 0;
};
