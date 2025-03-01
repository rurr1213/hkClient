#pragma once
#include "hkAPI.h"

class hkIServerService {
public:
    hkIServerService(hkAPI& api) : api(api) {}
    ~hkIServerService() {}
    virtual bool init() = 0;
    virtual bool deinit() = 0;
    virtual bool onPublishInfo(PublishInfo& publishInfo) = 0;
protected:
    hkAPI& api;
};

class hkServerServiceDefault : public hkIServerService {
    public:
        hkServerServiceDefault(hkAPI& api) : hkIServerService(api) {}
        ~hkServerServiceDefault() {}
        bool init() { return true;};
        bool deinit() { return true;};
        bool onPublishInfo(PublishInfo& publishInfo) { return true;};
};