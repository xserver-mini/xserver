/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/


#pragma once

#include "../utils/common.h"
#include "../robot/event.h"

#define BIND_EVENT(__CLASS__) bindEvent(__CLASS__::EEventID, (OnEventHandle)&Handle::On##__CLASS__)
#define HEADER_EVENT(__CLASS__) static void On##__CLASS__(Service&, const __CLASS__&)
#define SOURCE_EVENT(__CLASS__) void Handle::On##__CLASS__(Service& service, const __CLASS__& event)

class XRobot;
class XService
{
public:
	typedef void(*OnEventHandle)(XService&, const XEvent&);

	XService(XRobot* robot);
	virtual ~XService();
	static 	XService* New(XRobot* robot);

	virtual void onInit();
	virtual void onStart();
	virtual void onStop();

	void openLog(bool enable);
	void openCheckCost(bool enable);
	void setMinCostTime(int costTime);
	bool isRunning();

	virtual bool sendEvent(XEvent* event, int targetId = 0);
	virtual void onEvent(const XEvent& event);
	bool returnEvent(XEvent* event);

	//update
	void sendEventUpdate();
	virtual void onEventUpdate();

	//timer
	int64_t startEventTimer(int delay, int interval, bool isRepeat, int custom);
	void stopEventTimer(int64_t timerId = 0);
	virtual void onEventTimer(const XEventTimerMsg& event);

	const int robotId_;
	const std::string serviceName_;
protected:
	void bindEvent(int eventId, OnEventHandle handle);
	void sleep(int64_t msecond);
protected:
	int64_t timerId_;
	std::unordered_map<int, OnEventHandle> mapEventHandles_;
	const XEvent* focusEvent_;
private:
	XRobot* robot_;
	friend class XServiceSocket;
};