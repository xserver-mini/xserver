#pragma once

#include "core/robot/event.h"
#include "robotid.h"
#include "core/common.h"
#include <string>

//test
struct EventPing : public XEventMsg
{
    virtual ~EventPing() 
    {
        XINFO("=>");
    }
    enum {
        EEventID = 10001
    };
    virtual inline int getEventId() const
    {
        return EEventID;
    }
    virtual const char* getEventName() const
    {
        return "EventPing";
    }
};

struct EventPong : public XEventMsg
{
    virtual ~EventPong()
    {
        XINFO("=>");
    }
    enum {
        EEventID = 10002
    };
    virtual inline int getEventId() const
    {
        return EEventID;
    }
    virtual const char* getEventName() const
    {
        return "EventPong";
    }
};

