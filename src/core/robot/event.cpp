/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/

#include "event.h"
#include <cassert>

//XEvent
XEvent::XEvent()
	:uid_(0),
	ref_(1),
	srcId_(-1),
	dstId_(-1),
	eventId_(-1),
	sessionId_(0),
	eEventType_(EXEventTypeBase)
{
}

XEvent::~XEvent()
{
	assert(ref_ == 0);
}

void XEvent::retain()
{
	++ref_;
}

void XEvent::release()
{
	assert(ref_ > 0);
	--ref_;
	if (ref_ == 0)
	{
		delete this;
	}
}

int XEvent::getEventId() const
{
	return EEventID;
}

const char* XEvent::getEventName() const
{
	return "XEvent";
}


//XEventUpdate
XEventUpdate::XEventUpdate()
	:XEvent()
{
	eEventType_ = EXEventTypeUpdate;
}

int XEventUpdate::getEventId() const
{
	return EEventID;
}

const char* XEventUpdate::getEventName() const
{
	return "XEventUpdate";
}


//XEventTimer
XEventTimer::XEventTimer()
	:XEvent()
{
	eEventType_ = EXEventTypeTimer;
}

int XEventTimer::getEventId() const
{
	return EEventID;
}

const char* XEventTimer::getEventName() const
{
	return "XEventTimer";
}


//XEventTimerCtrl
XEventTimerCtrl::XEventTimerCtrl()
	:XEventTimer()
{
}

int XEventTimerCtrl::getEventId() const
{
	return EEventID;
}

const char* XEventTimerCtrl::getEventName() const
{
	return "XEventTimerCtrl";
}

//XEventTimerMsg
XEventTimerMsg::XEventTimerMsg()
	:XEventTimer()
{
}

int XEventTimerMsg::getEventId() const
{
	return EEventID;
}

const char* XEventTimerMsg::getEventName() const
{
	return "XEventTimerMsg";
}

//XEventMsg
XEventMsg::XEventMsg()
	:XEvent()
{
	eEventType_ = EXEventTypeMsg;
}

int XEventMsg::getEventId() const
{
	return EEventID;
}

const char* XEventMsg::getEventName() const
{
	return "XEventMsg";
}

