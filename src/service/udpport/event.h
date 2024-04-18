#pragma once

#include "utils/event.h"
#include <string>
#include "data.h"
#include "core/socket/servicesocket.h"

enum EUdpState
{
    EUdpStateNone,
    EUdpStateOpen,
    EUdpStateClose,
    EUdpStateConnect,
    EUdpStateSend,
    EUdpStateReceive,
    EUdpStateError
};

//EventUdpState
struct EventUdpState : public XEventMsg
{
    enum {
        EEventID = ERobotIDUdpPort * 10000 + 1
    };
    virtual inline int getEventId() const
    {
        return EEventID;
    }
    virtual const char* getEventName() const
    {
        return "EventUdpState";
    }

    EUdpState estate_;
    int64_t uuid_;
    XUdpAddress address_;
};

//EventUdpData
struct EventUdpData : public EventUdpState
{
    EventUdpData():buffer_(0){}
    virtual ~EventUdpData()
    {
        if (buffer_) delete buffer_;
        buffer_ = 0;
    }

    enum {
        EEventID = ERobotIDUdpPort * 10000 + 2
    };

    virtual inline int getEventId() const
    {
        return EEventID;
    }

    virtual const char* getEventName() const
    {
        return "EventUdpData";
    }

    std::string* getBuffer()
    {
        if (!buffer_) buffer_ = new std::string;
        return buffer_;
    }

    std::string* getBuffer() const
    {
        XASSERT(buffer_);
        return buffer_;
    }

    std::string* swap()
    {
        auto ret = buffer_;
        buffer_ = 0;
        return ret;
    }

    uint16_t msgId_;
protected:
    std::string* buffer_;
};

//EventUdpPackage
struct EventUdpPackage : public EventUdpState
{
    EventUdpPackage():package_(0){}
    virtual ~EventUdpPackage()
    {
        if (package_) delete package_;
        package_ = 0;
    }
    enum {
        EEventID = ERobotIDUdpPort * 10000 + 3
    };
    virtual inline int getEventId() const
    {
        return EEventID;
    }
    virtual const char* getEventName() const
    {
        return "EventUdpPackage";
    }

    UdpPackage* getPackage()
    {
        if (!package_) package_ = new UdpPackage;
        return package_;
    }
    UdpPackage* swap()
    {
        auto ret = package_;
        package_ = 0;
        return ret;
    }
protected:
    UdpPackage* package_;
};

//EventUdpPackages
struct EventUdpPackages : public EventUdpState
{
    EventUdpPackages(){}
    virtual ~EventUdpPackages()
    {
        for (size_t i = 0; i < vect_.size(); i++) delete vect_[i];
        vect_.clear();
    }
    enum {
        EEventID = ERobotIDUdpPort * 10000 + 4
    };
    virtual inline int getEventId() const
    {
        return EEventID;
    }
    virtual const char* getEventName() const
    {
        return "EventUdpPackages";
    }
    std::vector<UdpPackage*> vect_;
};

//EventUdpError
struct EventUdpError : public EventUdpState
{
    enum {
        EEventID = ERobotIDUdpPort * 10000 + 5
    };
    virtual inline int getEventId() const
    {
        return EEventID;
    }
    virtual const char* getEventName() const
    {
        return "EventUdpError";
    }
    uint16_t msgId_;
    std::string buffer_;
};