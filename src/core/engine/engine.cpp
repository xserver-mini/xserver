/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/

#include "engine.h"
#include "../utils/common.h"
#include "../robot/robot.h"
#include "../robot/event.h"

XEngine::XEngine(int engineId)
	:sendUid_(0),
	receiveUid_(0),
	engineId_(engineId),
	thread_(nullptr),
	isRunning_(false),
	isIdle_(true)
{
}

XEngine::~XEngine()
{
	assert(!thread_);
	while (!eventQueue_.empty())
	{
		delete eventQueue_.front();
		eventQueue_.pop();
	}
}

bool XEngine::sendEvent(XEvent* event)
{
	if (!event)
	{
		assert(false);
		return false;
	}
	lock_.lock();
	event->uid_ = sendUid_++;
	eventQueue_.push(event);
	lock_.unlock();
	cv_.notify_one();
	return true;
}

void XEngine::addRobot(XRobot* robot)
{
	if (thread_)
	{
		assert(false);
		return;
	}
	if (!robot)
	{
		assert(false);
		return;
	}
	assert(robot->engineId_ <= 0);

	auto iter = mapRobots_.find(robot->robotId_);
	if (iter != mapRobots_.end())
	{
		assert(false);
		return;
	}
	
	robot->engineId_ = engineId_;
	vectRobots_.push_back(robot);
	mapRobots_[robot->robotId_] = robot;
}

void XEngine::start()
{
	if (thread_)
	{
		assert(false);
		return;
	}
	isRunning_ = true;
	thread_ = new std::thread(XEngine::OnRun, this);
	//XLOG("");
}

void XEngine::wait()
{
	assert(thread_);
	if (thread_)
	{
		thread_->join();
	}
}

void XEngine::OnRun(XEngine* engine)
{
	if (!engine)
	{
		assert(false);
		return;
	}
	engine->onRun();
}

void XEngine::onRun()
{
	std::this_thread::sleep_for(std::chrono::seconds(1));
	auto curPid = std::this_thread::get_id();;
	auto thePid = thread_->get_id();
	if(curPid != thePid)
	{
		XASSERT(false);
	}
	//XLOG("entor");
	assert(!vectRobots_.empty());
	for (auto& iter : vectRobots_)
	{
		iter->onStart();
	}
	XEvent* event = 0;
	XRobot* robot = 0;
	while (isRunning_)
	{
		isIdle_ = true;
		std::unique_lock<std::mutex> lock(mutex_);
		cv_.wait(lock, [this] { return !eventQueue_.empty(); });
		isIdle_ = false;
		while(isRunning_)
		{
			lock_.lock();
			if(eventQueue_.empty())
			{
				lock_.unlock();
				break;
			}
			event = eventQueue_.front();
			eventQueue_.pop();
			lock_.unlock();
			if(!event)
			{
				continue;
			}
			if (event->uid_ != receiveUid_)
			{
				XERROR("occur error. receiveUid_=%ld, event->uid_=%ld", receiveUid_, event->uid_);
				assert(false);
			}
			receiveUid_++;
			auto iter = mapRobots_.find(event->dstId_);
			if (iter != mapRobots_.end())
			{
				robot = iter->second;
				if (robot && robot->engineId_ == engineId_)
				{
					robot->onEvent(*event);
				}
				else
				{
					assert(false);
				}
			}
			else
			{
				assert(false);
			}
			delete event;
		}
	}
	for (auto& iter : vectRobots_)
	{
		iter->onStop();
	}
	//XLOG("exit");
}

bool XEngine::checkThread()
{
	return currentThreadId() == theThreadId();
}

std::thread::id XEngine::currentThreadId()
{
	return std::this_thread::get_id();
}

std::thread::id XEngine::theThreadId()
{
	return thread_ ? thread_->get_id() : std::thread::id();
}

