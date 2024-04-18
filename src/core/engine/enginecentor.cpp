/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/

#include "enginecentor.h"
#include "engine.h"
#include "../utils/common.h"
#include "../service/servicecentor.h"
#include "../robot/robotcentor.h"
#include "../timer/servicetimer.h"
#include "../robot/event.h"

XEngineCentor* XEngineCentor::Instance_ = nullptr;

XEngineCentor& XEngineCentor::GetInstance()
{
	if (!Instance_)
	{
		Instance_ = new XEngineCentor;
	}
	return *Instance_;
}

XEngineCentor::XEngineCentor()
	:makeUid_(1),
	isLockCreate_(false)
{
	runThreadId_ = std::this_thread::get_id();
}

XEngineCentor::~XEngineCentor()
{
	checkThread();
	for (auto& engine : vectEngines_)
	{
		delete engine;
	}
	vectEngines_.clear();
}

void XEngineCentor::run()
{
	if (isLockCreate_)
	{
		assert(false);
		return;
	}
	checkThread();
	//XLOG("====>>");

	XServiceCentor::GetInstance().registerService<XServiceTimer>("Timer");
	XRobotCentor::GetInstance().createSingleRobot<XRobot>(EXRobotIDTimer, "Timer", false);

	XServiceCentor::GetInstance().start();
	XRobotCentor::GetInstance().start();
	isLockCreate_ = true;
	for (int i = 0; i < vectEngines_.size(); ++i)
	{
		auto& engine = vectEngines_[i];
		assert(engine->engineId_ == i + 1);
		engine->start();
	}
	auto sevent = new XEventAppStart;
	XRobotCentor::GetInstance().sendEvent(sevent, EXRobotIDMain, EXRobotIDMain);
	//XLOG("<<===");
}

void XEngineCentor::wait()
{
	for (auto& engine : vectEngines_)
	{
		engine->wait();
	}
}

XEngine* XEngineCentor::createEngine()
{
	if (isLockCreate_)
	{
		assert(false);
		return 0;
	}
	checkThread();
	auto engine = new XEngine(makeUid_++);
	vectEngines_.push_back(engine);
	assert(engine->engineId_ == vectEngines_.size());
	return engine;
}

XEngine* XEngineCentor::getEngine(int engineId)
{
	if (!isLockCreate_)
	{
		assert(false);
		return 0;
	}
	if (engineId <= 0)
	{
		assert(false);
		return 0;
	}
	if (engineId > vectEngines_.size())
	{
		assert(false);
		return 0;
	}
	auto engine = vectEngines_[engineId - 1];
	if (!engine)
	{
		assert(false);
		return 0;
	}
	assert(engine->engineId_ == engineId);
	return engine;

}

bool XEngineCentor::checkThread()
{
	assert(runThreadId_ == std::this_thread::get_id());
	return runThreadId_ == std::this_thread::get_id();
}
