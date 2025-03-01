#pragma once

#include "hkDeviceMgrI.h"

class hkAPI {
public:
    virtual void publish() = 0;
    virtual void publishAck() = 0;
    virtual void subscribe() = 0;
    virtual void unsubscribe() = 0;
    virtual void createGroup() = 0;
    virtual void destroyGroup() = 0;
};

