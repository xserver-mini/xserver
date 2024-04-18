/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/


#pragma once

#include <map>
#include "../service/service.h"
#include "../utils/spinlock.h"

class XServiceTimer : public XService
{
public:
    struct TimerEvent
    {
        TimerEvent();
        bool isRepeat_;
        int robotId_;
        int custom_;
        int64_t uid_;
        int delay_;
        int interval_;

        int64_t startTS_;
        int64_t nextTS_;
        int times_;
    };

    XServiceTimer(XRobot* robot);
	virtual ~XServiceTimer();
	static 	XServiceTimer* New(XRobot* robot);

	virtual void onInit();
	virtual void onStart();
	virtual void onStop();

    virtual void onEventUpdate();
    virtual void onEvent(const XEvent& event);

    void fireTimer(int64_t timerId);
    TimerEvent& createTimer(int64_t timerId);
    void startTimer(TimerEvent& timerEvent);
    void stopTimer(int64_t timerId);

    static int64_t allocTimerId();
protected:
    static int64_t Uid_;
    static XSpinlock Lock_;

    std::unordered_map<int64_t, TimerEvent> mapTimers_;
    std::multimap<int64_t, int64_t> mapQueue_;
    std::vector<int64_t> vectTimerIds_;
};
