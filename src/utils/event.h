#pragma once

#include "core/robot/event.h"
#include "robotid.h"
#include <string>

//test
struct EventPingPong : public XEventMsg
{
    enum {
        EEventID = 10001
    };
    virtual inline int getEventId() const
    {
        return EEventID;
    }
};


