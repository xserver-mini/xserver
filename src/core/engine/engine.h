/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/

#pragma once

#include <iostream>
#include <queue>
#include <thread>
#include <unordered_map>
#include <condition_variable>
#include "../utils/spinlock.h"

class XEvent;
class XRobot;
class XEngine
{
public:
	XEngine(int engineId);
	virtual ~XEngine();
	void start();
	void wait();

	bool sendEvent(XEvent* event);
	void addRobot(XRobot* robot);

	bool checkThread();
	std::thread::id currentThreadId();
	std::thread::id theThreadId();
	inline bool isIdle() { return isIdle_; }
	inline bool isRunning() { return isRunning_; }
	inline bool isSingle() { return vectRobots_.size() == 1; }
	void robotAwait();
	void robotWakeUp();
	
	bool isAwait_;
	const int engineId_;
protected:
	static void OnRun(XEngine* engine);
	void onRun();

	int64_t sendUid_;
	int64_t receiveUid_;
	XSpinlock lock_;
	std::queue<XEvent*> eventQueue_;

	std::vector<XRobot*> vectRobots_;
	std::unordered_map<int, XRobot*> mapRobots_;

	bool isIdle_;
	bool isRunning_;
	std::thread* thread_;
	std::mutex mutex_;
	std::condition_variable cv_;
};