#pragma once


class IHKDeviceMgr
{
public:
	virtual bool onOpenForDataEvent(void) = 0;
	virtual bool onClosedForDataEvent(void) = 0;
	virtual bool onReceivedDataEvent(void) = 0;
};
