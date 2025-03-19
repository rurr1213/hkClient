#pragma once
#include "hkAPI.h"

class HKIClientService {
public:
    HKIClientService(HKIAPI& api) : api(api) {}
    ~HKIClientService() {}
    virtual bool init() = 0;
    virtual bool deinit() = 0;
    virtual bool onPublishInfo(PublishInfo& publishInfo) = 0;
protected:
    HKIAPI& api;
};

class hkIClientServiceDefault : public HKIClientService {
    public:
        hkIClientServiceDefault(HKIAPI& api) : HKIClientService(api) {}
        ~hkIClientServiceDefault() {}
        bool init() { return true;};
        bool deinit() { return true;};
        bool onPublishInfo(PublishInfo& publishInfo) { return true;};
};