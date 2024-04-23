/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/

#include "service.h"
#include "../robot/robot.h"
#include "../robot/event.h"
#include "../timer/servicetimer.h"

XService::XService(XRobot* robot)
	:robot_(robot),
    timerId_(-1),
    focusEvent_(0),
    robotId_(robot ? robot->robotId_ : -1),
    serviceName_(robot ? robot->serviceName() : "XService")
{
	assert(robot);
}

XService::~XService()
{
}

XService* XService::New(XRobot* robot)
{
	return new XService(robot);
}

void XService::openLog(bool enable)
{
    if (!robot_)
    {
        XASSERT(false);
        return;
    }
    robot_->isLog_ = enable;
}

void XService::openCheckCost(bool enable)
{
    if (!robot_)
    {
        XASSERT(false);
        return;
    }
    robot_->isCostTime_ = enable;
}

void XService::setMinCostTime(int costTime)
{
    if (!robot_)
    {
        XASSERT(false);
        return;
    }
    robot_->minCostTime_ = costTime;
}

bool XService::isRunning()
{
    if (!robot_)
    {
        XASSERT(false);
        return false;
    }
    return robot_->isRunning();
}

void XService::onInit()
{
    XDEBUG("===>>");
}

void XService::onStart()
{
    XDEBUG("===>>");
}

void XService::onStop()
{
    XDEBUG("===>>");
}

bool XService::sendEvent(XEvent* event, int targetId)
{
    if (!robot_)
    {
        XERROR("robot_==null. %s[%d] targetId=%d", serviceName_.data(), robotId_, targetId);
        assert(false);
        return false;
    }
    return robot_->sendEvent(event, targetId);
}

bool XService::await()
{
    if (!robot_)
    {
        XASSERT(false);
        return false;
    }
    if (robot_->isWaiting())
    {
        XASSERT(false);
        return false;
    }
    return robot_->await();
}

void XService::wakeUp()
{
    if (!robot_)
    {
        XASSERT(false);
        return ;
    }
    robot_->wakeUp();
}

bool XService::isWaiting()
{
    if (!robot_)
    {
        XASSERT(false);
        return false;
    }
    return robot_->isWaiting();
}

bool XService::returnEvent(XEvent* event)
{
    if (!focusEvent_)
    {
        XASSERT(false);
        return false;
    }
    auto dstId = focusEvent_->srcId_;
    if (dstId <= 0)
    {
        XASSERT(false);
        return false;
    }
    return sendEvent(event, dstId);
}

void XService::onEvent(const XEvent& event)
{
    if (!isRunning())
    {
        XERROR("robot_ is running. %s[%d] eventId=%d", serviceName_.data(), robotId_, event.getEventId());
        XASSERT(false);
        return;
    }
    XASSERT(!focusEvent_);
    focusEvent_ = &event;
    if (event.eEventType_ == EXEventTypeUpdate)
    {
        onEventUpdate();
        focusEvent_ = 0;
        return;
    }
    if (event.eEventType_ == EXEventTypeTimer)
    {
        if (event.getEventId() != XEventTimerMsg::EEventID)
        {
            XASSERT(false);
            focusEvent_ = 0;
            return;
        }
        const XEventTimerMsg* timeEvent = dynamic_cast<const XEventTimerMsg*>(&event);
        if (!timeEvent)
        {
            XASSERT(false);
            focusEvent_ = 0;
            return;
        }
        timerId_ = timeEvent->timerId_;
        onEventTimer(*timeEvent);
        focusEvent_ = 0;
        return;
    }

    auto eventId = event.getEventId();
    auto iter = mapEventHandles_.find(eventId);
    if (iter == mapEventHandles_.end())
    {
        XERROR("event no exist. %s[%d] eventId=%d", serviceName_.data(), robotId_, eventId);
        assert(false);
        focusEvent_ = 0;
        return;
    }
    iter->second(*this, event);
    focusEvent_ = 0;
}

void XService::bindEvent(int eventId, OnEventHandle handle)
{
    if (isRunning())
    {
        XERROR("robot_ is running. %s[%d] eventId=%d", serviceName_.data(), robotId_, eventId);
        assert(false);
        return;
    }
    auto iter = mapEventHandles_.find(eventId);
    if (iter != mapEventHandles_.end())
    {
        XERROR("repeat register handle. %s[%d] eventId=%d", serviceName_.data(), robotId_, eventId);
        assert(false);
        return;
    }
    //XLOG("%s[%d] eventId=%d", serviceName_.data(), robotId_, eventId);
    mapEventHandles_[eventId] = handle;
}

void XService::sleep(int64_t msecond)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(msecond));
}

void XService::sendEventUpdate()
{
    auto event = new XEventUpdate;
    sendEvent(event);
}

void XService::onEventUpdate()
{

}

int64_t XService::startEventTimer(int delay, int interval, bool isRepeat, int custom)
{
    //XINFO("startEventTimer delay=%d, interval=%d, isRepeat=%d, custom=%d", delay, interval, isRepeat, custom);
    if (timerId_ > 0)
    {
        stopEventTimer(timerId_);
        timerId_ = -1;
    }
    timerId_ = XServiceTimer::allocTimerId();
    auto event = new XEventTimerCtrl;
    event->timerId_ = timerId_;
    event->delay_ = delay;
    event->interval_ = interval;
    event->isRepeat_ = isRepeat;
    event->custom_ = custom;
    sendEvent(event, EXRobotIDTimer);
    return timerId_;
}

void XService::stopEventTimer(int64_t timerId)
{
    if (!timerId)
    {
        if (timerId_ <= 0)
        {
            return;
        }
        timerId = timerId_;
    }
    if (timerId == timerId_)
    {
        timerId_ = -1;
    }
    auto event = new XEventTimerMsg;
    event->timerId_ = timerId;
    sendEvent(event, EXRobotIDTimer);
}

void XService::onEventTimer(const XEventTimerMsg& event)
{

}
