#pragma once
#include "hkAPI.h"

class HKIServerService {
public:
    HKIServerService(HKIAPI& api) : hkAPI(api) {}
    ~HKIServerService() {}
    virtual bool init(void) = 0;
    virtual bool deinit(void) = 0;
    virtual bool onPublishInfo(PublishInfo& publishInfo) = 0;
protected:
    HKIAPI& hkAPI;
};

class hkServerServiceDefault : public HKIServerService {
    public:
        hkServerServiceDefault(HKIAPI& api) : HKIServerService(api) {}
        ~hkServerServiceDefault() {}
        bool init() { return true;};
        bool deinit() { return true;};
        bool onPublishInfo(PublishInfo& publishInfo) { return true;};
};