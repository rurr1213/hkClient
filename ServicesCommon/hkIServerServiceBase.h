#pragma once
#include <string>

class HKIAPI;
class PublishInfo;

class HKIServerServiceBase {
public:
    std::string registeredGroupName;
    HKIServerServiceBase(HKIAPI& api) : hkAPI(api) {}
    ~HKIServerServiceBase() {}
    virtual bool init(void);
    virtual bool deinit(void);
    virtual bool onPublishInfo(PublishInfo& publishInfo) = 0;
protected:
    HKIAPI& hkAPI;
};

class hkServerServiceDefault : public HKIServerServiceBase {
    public:
        hkServerServiceDefault(HKIAPI& api) : HKIServerServiceBase(api) {}
        ~hkServerServiceDefault() {}
        bool init() { return true;};
        bool deinit() { return true;};
        bool onPublishInfo(PublishInfo& publishInfo) { return true;};
};

