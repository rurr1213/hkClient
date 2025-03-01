#pragma once
#include "hkAPI.h"

class hkIClientService {
public:
    hkIClientService(hkAPI& api) : api(api) {}
    ~hkIClientService() {}
    virtual bool init() = 0;
    virtual bool deinit() = 0;
    virtual bool onPublishInfo(PublishInfo& publishInfo) = 0;
protected:
    hkAPI& api;
};

class hkIClientServiceDefault : public hkIClientService {
    public:
        hkIClientServiceDefault(hkAPI& api) : hkIClientService(api) {}
        ~hkIClientServiceDefault() {}
        bool init() { return true;};
        bool deinit() { return true;};
        bool onPublishInfo(PublishInfo& publishInfo) { return true;};
};