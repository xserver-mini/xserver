/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/


#pragma once

#include "robot/event.h"
#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>
#include "utils/spinlock.h"

class XApp
{
public:
	XApp();
	virtual ~XApp();
	static XApp& GetInstance();

	virtual void start();
	virtual void run();

	void startAsync();
	static void OnStartAsync(XApp* app);

	void onEvent(XEvent* event);
	bool sendEvent(XEvent* event, int dstId);

	void doMainThread();
	void onMainThread();
	virtual void onMainThreadEvent(XEvent* event);
	void setHandle(void (*handle)(const XEvent&));

private:
	static XApp* CreateApp();
	static XApp* Instance_;

protected:
	bool isRunning_;
	int64_t sendUid_;
	int64_t receiveUid_;
	XSpinlock lock_;
	std::queue<XEvent*> eventQueue_;
	void (*handle_)(const XEvent&);

	std::mutex mutex_;
	std::condition_variable cv_;
};