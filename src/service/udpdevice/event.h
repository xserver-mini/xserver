#pragma once

#include "utils/event.h"
#include "data.h"

//udp
struct EventUdpCtrl : public XEventMsg
{
    enum {
        EEventID = ERobotIDUdpDevice * 10000 + 1
    };
    virtual inline int getEventId() const
    {
        return EEventID;
    }
    virtual const char* getEventName() const
    {
        return "EventUdpCtrl";
    }
};
