/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/


#include "servicetimer.h"
#include "../robot/event.h"


XServiceTimer::TimerEvent::TimerEvent()
    :isRepeat_(false),
    robotId_(0),
    custom_(0),
    uid_(0),
    delay_(0),
    interval_(0),
    startTS_(0),
    nextTS_(0),
    times_(0)
{
}

XServiceTimer::XServiceTimer(XRobot* robot)
	:XService(robot)
{
    assert(robot);
    openLog(false);
    openCheckCost(false);
}

XServiceTimer::~XServiceTimer()
{
}

XServiceTimer* XServiceTimer::New(XRobot* robot)
{
	return new XServiceTimer(robot);
}

void XServiceTimer::onInit()
{
    //XDEBUG("===>>");
}

void XServiceTimer::onStart()
{
    //XDEBUG("===>>");
    sendEventUpdate();
}

void XServiceTimer::onStop()
{
    //XDEBUG("===>>");
}

void XServiceTimer::onEvent(const XEvent& event)
{
    if (!isRunning())
    {
        XERROR("robot_ is running. %s[%d] eventId=%d", serviceName_.data(), robotId_, event.getEventId());
        assert(false);
        return;
    }
    if (event.eEventType_ == EXEventTypeTimer)
    {
        if (event.getEventId() == XEventTimerCtrl::EEventID)
        {
            const XEventTimerCtrl* timeEvent = dynamic_cast<const XEventTimerCtrl*>(&event);
            if (!timeEvent)
            {
                XASSERT(false);
                return;
            }
            auto& timerEvent = createTimer(timeEvent->timerId_);
            timerEvent.isRepeat_ = timeEvent->isRepeat_;
            timerEvent.delay_    = timeEvent->delay_;
            timerEvent.interval_ = timeEvent->interval_;
            timerEvent.custom_   = timeEvent->custom_;
            timerEvent.robotId_  = timeEvent->srcId_;
            startTimer(timerEvent);
        }
        else  if (event.getEventId() == XEventTimerMsg::EEventID)
        {
            const XEventTimerMsg* timeEvent = dynamic_cast<const XEventTimerMsg*>(&event);
            if (!timeEvent)
            {
                XASSERT(false);
                return;
            }
            stopTimer(timeEvent->timerId_);
        }
        else
        {
            XASSERT(false);
        }
        return;
    }
    XService::onEvent(event);
}

void XServiceTimer::onEventUpdate()
{
    assert(vectTimerIds_.empty());
    if(!mapQueue_.empty())
    {
        int64_t currentTime = open::OpenTime::MilliUnixtime();
        while(!mapQueue_.empty())
        {
            auto iter = mapQueue_.begin();
            if(currentTime >= iter->first)
            {
                vectTimerIds_.push_back(iter->second);
                mapQueue_.erase(iter);
            }
            else
            {
                break;
            }
        }
        if(!vectTimerIds_.empty())
        {
            for(auto& timerId : vectTimerIds_)
            {
                fireTimer(timerId);
            }
            vectTimerIds_.clear();
        }
    }
    this->sleep(10);
    sendEventUpdate();
}

void XServiceTimer::fireTimer(int64_t timerId)
{
    auto iter = mapTimers_.find(timerId);
    if (iter == mapTimers_.end())
    {
        return;
    }
    TimerEvent& timerEvent = iter->second;
    assert(timerId == timerEvent.uid_);

    if (timerEvent.startTS_ > 0)
    {
        timerEvent.startTS_ = 0;
        mapQueue_.insert({ timerEvent.nextTS_, timerEvent.uid_ });
    }
    else
    {
        timerEvent.times_++;

        auto event = new XEventTimerMsg;
        //event->timeStamp_ = timerEvent.nextTS_;
        event->timerId_ = timerEvent.uid_;
        event->custom_ = timerEvent.custom_;
        event->times_ = timerEvent.times_;
        event->interval_ = timerEvent.interval_;
        sendEvent(event, timerEvent.robotId_);

        if (timerEvent.isRepeat_)
        {
            timerEvent.nextTS_ += timerEvent.interval_;
            mapQueue_.insert({ timerEvent.nextTS_, timerEvent.uid_ });
        }
    }
}

XServiceTimer::TimerEvent& XServiceTimer::createTimer(int64_t timerId)
{
    auto iter = mapTimers_.find(timerId);
    if (iter != mapTimers_.end())
    {
        XASSERT(false);
    }
    auto& timerEvent = mapTimers_[timerId];
    timerEvent.uid_ = timerId;
    timerEvent.robotId_ = 0;
    return timerEvent;
}

void XServiceTimer::startTimer(TimerEvent& event)
{
    auto iter = mapTimers_.find(event.uid_);
    if (iter == mapTimers_.end())
    {
        assert(false);
        return;
    }
    TimerEvent& timerEvent = iter->second;
    assert(timerEvent.robotId_ > 0);
    timerEvent.times_ = 0;
    if (timerEvent.delay_ > 0)
    {
        timerEvent.startTS_ = open::OpenTime::MilliUnixtime() + timerEvent.delay_;
        timerEvent.nextTS_ = timerEvent.startTS_ + timerEvent.interval_;
        mapQueue_.insert({ timerEvent.startTS_, timerEvent.uid_ });
    }
    else
    {
        timerEvent.startTS_ = 0;
        timerEvent.nextTS_ = open::OpenTime::MilliUnixtime() + timerEvent.interval_;
        mapQueue_.insert({ timerEvent.nextTS_, timerEvent.uid_ });
    }
}

void XServiceTimer::stopTimer(int64_t timerId)
{
    auto iter = mapTimers_.find(timerId);
    if (iter == mapTimers_.end())
    {
        //assert(false);
        return;
    }
    TimerEvent& timerEvent = iter->second;
    int64_t key = timerEvent.startTS_;
    if (key == 0)
    {
        key = timerEvent.nextTS_;
    }
    auto iterQ = mapQueue_.find(key);
    while (iterQ != mapQueue_.end() && iterQ->first == key)
    {
        if (iterQ->second == timerId)
        {
            iterQ = mapQueue_.erase(iterQ);
        }
        else
        {
            iterQ++;
        }
    }
    mapTimers_.erase(iter);
}

int64_t XServiceTimer::Uid_ = 1;
XSpinlock XServiceTimer::Lock_;

int64_t XServiceTimer::allocTimerId()
{
    int64_t timerId = 0;
    Lock_.lock();
    if (Uid_ <= 0)
    {
        Uid_ = 1;
    }
    timerId = Uid_++;
    Lock_.unlock();
    return timerId;
}
