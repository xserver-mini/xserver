/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/


#include "app.h"
#include <thread>
#include <cassert>
#include "engine/enginecentor.h"
#include "robot/robotcentor.h"

XApp* XApp::Instance_ = nullptr;

XApp& XApp::GetInstance()
{
	if (!Instance_)
	{
		Instance_ = CreateApp();
	}
	return *Instance_;
}

XApp::XApp()
	:sendUid_(0),
	receiveUid_(0),
	handle_(0),
	isRunning_(false)
{
	uint32_t x = 1;
	uint8_t* c = (uint8_t*)&x;
	assert(*c == 1);
}

XApp::~XApp()
{
}

void XApp::start()
{
}

void XApp::startAsync()
{
	std::thread th(XApp::OnStartAsync, this);
	th.detach();
}

void XApp::OnStartAsync(XApp* app)
{
	if (!app)
	{
		assert(false);
		return;
	}
	app->start();
	app->run();
}

void XApp::run()
{
	XASSERT(!isRunning_);
	XEngineCentor::GetInstance().run();
	isRunning_ = true;
	while (isRunning_)
	{
		onMainThread();
		std::unique_lock<std::mutex> lock(mutex_);
		cv_.wait(lock);
	}
	XEngineCentor::GetInstance().wait();
}

void XApp::onEvent(XEvent* event)
{
	if (!event)
	{
		assert(false);
		return;
	}
	if (event->dstId_ != EXRobotIDMain)
	{
		event->release();
		assert(false);
		return;
	}

	lock_.lock();
	event->uid_ = sendUid_++;
	eventQueue_.push(event);
	lock_.unlock();

	doMainThread();
}

bool XApp::sendEvent(XEvent* event, int dstId)
{
	auto& robotCentor = XRobotCentor::GetInstance();
	return robotCentor.sendEvent(event, dstId, EXRobotIDMain);
}

void XApp::doMainThread()
{
	cv_.notify_one();
}

void XApp::onMainThread()
{
	XEvent* event = 0;
	while (true)
	{
		lock_.lock();
		if (eventQueue_.empty())
		{
			lock_.unlock();
			break;
		}
		event = eventQueue_.front();
		eventQueue_.pop();
		lock_.unlock();
		if (!event)
		{
			continue;
		}
		if (event->uid_ != receiveUid_)
		{
			XERROR("occur error. receiveUid_=%ld, event->uid_=%ld", receiveUid_, event->uid_);
			assert(false);
		}
		receiveUid_++;
		onMainThreadEvent(event);
		event->release();
	}
}

void XApp::setHandle(void (*handle)(const XEvent&))
{
	handle_ = handle;
}

void XApp::onMainThreadEvent(XEvent* event)
{
	XASSERT(false);
}


