/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/

#include "robotcentor.h"
#include <cassert>
#include <iostream>
#include "event.h"
#include "../app.h"

XRobotCentor* XRobotCentor::Instance_ = nullptr;

XRobotCentor& XRobotCentor::GetInstance()
{
	if (!Instance_)
	{
		Instance_ = new XRobotCentor;
	}
	return *Instance_;
}

XRobotCentor::XRobotCentor()
    :isLockCreate_(false)
{
}

XRobotCentor::~XRobotCentor()
{
    for (auto& iter : mapRobots_)
    {
        delete iter.second;
    }
    mapRobots_.clear();
}

void XRobotCentor::start()
{
    assert(!isLockCreate_);
    isLockCreate_ = true;
}

bool XRobotCentor::sendEvent(XEvent* event, int dstId, int srcId)
{
    event->srcId_ = srcId;
    event->dstId_ = dstId;
    return sendEventInter(event);
}

bool XRobotCentor::sendEventInter(XEvent* event)
{
    //std::cout << "XRobotCentor::sendEvent " << " [" << event->srcId_ << "=>" << event->dstId_ << "]==>> " << std::endl;
    if (!event)
    {
        XERROR("event == 0.");
        assert(false);
        return false;
    }
    do {
        if (!isLockCreate_)
        {
            XERROR("[%d => %d] Create is lock.", event->srcId_, event->dstId_);
            assert(false);
            break;
        }

        if (event->dstId_ == EXRobotIDMain)
        {
            XApp::GetInstance().onEvent(event);
            return true;
        }

        auto iter = mapRobots_.find(event->dstId_);
        if (iter == mapRobots_.end())
        {
            XERROR("[%d => %d] Can't find robot.", event->srcId_, event->dstId_);
            assert(false);
            break;
        }
        auto robot = iter->second;
        if (!robot)
        {
            XERROR("[%d => %d] robot == 0.", event->srcId_, event->dstId_);
            assert(false);
            break;
        }
        if (robot->engineId_ <= 0)
        {
            XERROR("[%d => %d] Robot don't have engine.", event->srcId_, event->dstId_);
            assert(false);
            break;
        }
        auto engine = XEngineCentor::GetInstance().getEngine(robot->engineId_);
        if (!engine)
        {
            XERROR("[%d => %d] engine == 0.", event->srcId_, event->dstId_);
            assert(false);
            break;
        }
        return engine->sendEvent(event);
    } while (false);
    event->release();
    return false;
}

const std::string& XRobotCentor::getServiceName(int robotId)
{
    if (!isLockCreate_)
    {
        XERROR("Need lock create.");
        assert(false);
        static const std::string robotName = "Error";
        return robotName;
    }
    if (robotId < 0)
    {
        if (robotId == EXRobotIDSocket)
        {
            static const std::string robotName = "Socket";
            return robotName;
        }
        if (robotId == EXRobotIDMain)
        {
            static const std::string robotName = "Main";
            return robotName;
        }
        if (robotId == EXRobotIDCall)
        {
            static const std::string robotName = "Call";
            return robotName;
        }
        if (robotId == EXRobotIDOther)
        {
            static const std::string robotName = "Other";
            return robotName;
        }
        static const std::string robotName = "Unknown";
        return robotName;
    }
    auto iter = mapRobots_.find(robotId);
    if (iter == mapRobots_.end())
    {
        static const std::string robotName = "Empty";
        return robotName;
    }
    auto robot = iter->second;
    if (!robot)
    {
        static const std::string robotName = "Null";
        return robotName;
    }
    return robot->serviceName();
}
