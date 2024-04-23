/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/

#pragma once

#include <cstdint>

enum EXRobotID
{
	EXRobotIDOther  = -4,
	EXRobotIDCall   = -3,
	EXRobotIDSocket = -2,
	EXRobotIDMain   = -1,
	EXRobotIDSelf	= 0,
	EXRobotIDTimer  = 1,
};

enum EXEventType
{
	EXEventTypeBase,
	EXEventTypeUpdate,
	EXEventTypeSocket,
	EXEventTypeTimer,
	EXEventTypeMsg,
};

//XEvent
class XEvent
{
public:
	enum {
		EEventID = 1
	};
	XEvent();
	virtual ~XEvent();
	void retain();
	void release();

	virtual int getEventId() const;
	virtual const char* getEventName() const;

	int srcId_;
	int dstId_;
	int sessionId_;
	long long uid_;
	int eventId_;
	short ref_;
	EXEventType eEventType_;
};

//XEventUpdate
class XEventUpdate : public XEvent
{
public:
	enum {
		EEventID = 1
	};
	XEventUpdate();

	virtual int getEventId() const;
	virtual const char* getEventName() const;
};

//XEventTimer
class XEventTimer : public XEvent
{
public:
	enum {
		EEventID = 1
	};
	XEventTimer();
	virtual int getEventId() const;
	virtual const char* getEventName() const;
};

struct XEventTimerCtrl : public XEventTimer
{
	XEventTimerCtrl();
	enum {
		EEventID = 1
	};
	virtual int getEventId() const;
	virtual const char* getEventName() const;

	int64_t timerId_ = 0;
	bool isRepeat_ = 0;
	int delay_ = 0;
	int interval_ = 0;
	int custom_ = 0;
};

struct XEventTimerMsg : public XEventTimer
{
	XEventTimerMsg();
	enum {
		EEventID = 2
	};
	virtual int getEventId() const;
	virtual const char* getEventName() const;

	int64_t timerId_ = 0;
	int custom_ = 0;
	int times_  = 0;
	int interval_ = 0;
};

//XEventMsg
class XEventMsg : public XEvent
{
public:
	enum {
		EEventID = 1
	};
	XEventMsg();
	virtual int getEventId() const;
	virtual const char* getEventName() const;
};

// XEventAppStart
struct XEventAppStart : public XEventMsg
{
	enum {
		EEventID = 10002
	};
	virtual inline int getEventId() const
	{
		return EEventID;
	}
	virtual const char* getEventName() const
	{
		return "XEventAppStart";
	}
};
