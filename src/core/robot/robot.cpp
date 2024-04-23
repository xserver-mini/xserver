/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/

#include "robot.h"
#include "event.h"
#include "robotcentor.h"
#include "../engine/engine.h"
#include "../engine/enginecentor.h"
#include "../service/servicecentor.h"
#include "../common.h"

XRobot::XRobot(int robotId)
	:robotId_(robotId),
	service_(0),
	engineId_(0),
	isRunning_(false),
	isLog_(false),
	isCostTime_(true),
	server_(0),
	minCostTime_(30),
	isAwait_(false)
{
}

XRobot::~XRobot()
{
}

void XRobot::onInit()
{
	assert(!service_);
	if (!service_)
	{
		service_ = XServiceCentor::GetInstance().createService(this);
	}
	if (!service_)
	{
		assert(false);
		return;
	}
	service_->onInit();
}

void XRobot::onStart()
{
	if (!service_)
	{
		assert(false);
		return;
	}
	isRunning_ = true;
	service_->onStart();
}

void XRobot::onStop()
{
	if (!service_)
	{
		assert(false);
		return;
	}
	isRunning_ = false;
	service_->onStop();
}

bool XRobot::sendEvent(XEvent* event, int targetId)
{
	if (!event)
	{
		XERROR("[%s]event == NULL.", serviceName_.data());
		assert(false);
		return false;
	}
	if (targetId == EXRobotIDSelf)
	{
		targetId = robotId_;
	}
	event->srcId_ = robotId_;
	event->dstId_ = targetId;
	bool ret = false;
	if (event->srcId_ != event->dstId_)
	{
		ret = XRobotCentor::GetInstance().sendEventInter(event);
	}
	else
	{
		auto engine = XEngineCentor::GetInstance().getEngine(engineId_);
		if (!engine)
		{
			XERROR("[%s][%s][%d => %d] engine == 0. engineId_=%d", serviceName_.data(), event->getEventName(), event->srcId_, event->dstId_, engineId_);
			delete event;
			assert(false);
			return false;
		}
		ret = engine->sendEvent(event);
	}
	if (isLog_)
	{
		auto srcName = XRobotCentor::GetInstance().getServiceName(event->srcId_);
		auto dstName = XRobotCentor::GetInstance().getServiceName(event->dstId_);
		XLOG("%d[%s:%d] [%s:%d => %s:%d]%s", engineId_, serviceName_.data(), robotId_, 
			srcName.data(), event->srcId_, dstName.data(), event->dstId_, event->getEventName());
	}
	return ret;
}

void XRobot::onEvent(const XEvent& event)
{
	if (!service_)
	{
		assert(false);
		return;
	}
	if (isLog_ && event.srcId_ != EXRobotIDTimer)
	{
		auto srcName = XRobotCentor::GetInstance().getServiceName(event.srcId_);
		auto dstName = XRobotCentor::GetInstance().getServiceName(event.dstId_);
		XLOG("%d[%s:%d][%s:%d => %s:%d]%s", engineId_, serviceName_.data(), robotId_, 
			srcName.data(), event.srcId_, dstName.data(), event.dstId_, event.getEventName());
	}
	if (!isRunning_)
	{
		assert(false);
		return;
	}
	if (event.dstId_ != robotId_)
	{
		XERROR("target error. robotId_=%d, event.dstId=%d", robotId_, event.dstId_);
		assert(false);
		return;
	}
	if (isCostTime_)
	{
		auto duration = std::chrono::system_clock::now().time_since_epoch();
		auto startTS = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
		service_->onEvent(event);

		duration = std::chrono::system_clock::now().time_since_epoch();
		auto currentTS = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
		auto costTS = currentTS - startTS;
		if (costTS > minCostTime_)
		{
			XWARN("task cost too much time:%ld", costTS);
		}
	}
	else
	{
		service_->onEvent(event);
	}
}

bool XRobot::isIdle()
{
	auto engine = XEngineCentor::GetInstance().getEngine(engineId_);
	if (engine)
	{
		return engine->isIdle();
	}
	return false;
}

const std::string& XRobot::serviceName()
{
	if (serviceName_.empty())
	{
		static std::string Name_ = "XRobot";
		return Name_;
	}
	return serviceName_;
}

void XRobot::setServiceName(const std::string& name)
{
	serviceName_ = name;
}

void XRobot::wakeUp()
{
	if (isAwait_)
	{
		auto engine = XEngineCentor::GetInstance().getEngine(engineId_);
		if (!engine)
		{
			XASSERT(false);
			return;
		}
		if (!engine->isSingle())
		{
			XASSERT(false);
			return;
		}
		engine->robotWakeUp();
		isAwait_ = false;
	}
}

bool XRobot::await()
{
	auto engine = XEngineCentor::GetInstance().getEngine(engineId_);
	if (!engine)
	{
		XASSERT(false);
		return false;
	}
	if (!engine->isSingle())
	{
		XASSERT(false);
		return false;
	}
	isAwait_ = true;
	engine->robotAwait();
	return true;
}
