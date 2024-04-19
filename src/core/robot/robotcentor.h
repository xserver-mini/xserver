/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/


#pragma once

#include "robot.h"
#include "../engine/engine.h"
#include "../engine/enginecentor.h"

class XRobotCentor
{
public:
	XRobotCentor();
	virtual ~XRobotCentor();
	static XRobotCentor& GetInstance();

    void start();
    bool sendEvent(XEvent* event, int dstId, int srcId);
    bool sendEventInter(XEvent* event);

    template <class T>
    XRobot* createRobot(int robotId, const std::string& serviceName, bool isLog = false)
    {
        XEngineCentor::GetInstance().checkThread();
        if (isLockCreate_)
        {
            XERROR("XRobotCentor::createRobot Create is lock. serviceName=%s, robotId=%d", serviceName.data(), robotId);
            assert(false);
            return nullptr;
        }
        auto iter = mapRobots_.find(robotId);
        if (iter != mapRobots_.end())
        {
            XERROR("XRobotCentor::createRobot Repeat. serviceName=%s, robotId=%d", serviceName.data(), robotId);
            assert(false);
            return iter->second;
        }

        XRobot* robot = new T(robotId);
        robot->setServiceName(serviceName);
        robot->onInit();
        robot->isLog_ = isLog;
        mapRobots_[robotId] = robot;
        return robot;
    }


    template <class T>
    XRobot* createRobot(XEngine* engine, int robotId, const std::string& serviceName, bool isLog = false)
    {
        if (!engine)
        {
            assert(false);
            return nullptr;
        }
        auto robot = createRobot<T>(robotId, serviceName, isLog);
        if (robot)
        {
            engine->addRobot(robot);
        }
        return robot;
    }

    template <class T>
    XRobot* createSingleRobot(int robotId, const std::string& serviceName, bool isLog = false)
    {
        auto robot = createRobot<T>(robotId, serviceName, isLog);
        if (robot)
        {
            auto engine = XEngineCentor::GetInstance().createEngine();
            if (!engine)
            {
                assert(false);
            }
            else
            {
                engine->addRobot(robot);
            }
        }
        return robot;
    }

    const std::string& getServiceName(int robotId);
protected:
	volatile bool isLockCreate_;
	std::unordered_map<int, XRobot*> mapRobots_;
	static XRobotCentor* Instance_;

    friend class XRobot;
    friend class XSocketCentor;
};
