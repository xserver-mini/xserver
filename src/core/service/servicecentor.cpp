/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/


#include "servicecentor.h"
#include "../robot/robot.h"

XServiceCentor* XServiceCentor::Instance_ = nullptr;

XServiceCentor& XServiceCentor::GetInstance()
{
	if (!Instance_)
	{
		Instance_ = new XServiceCentor;
	}
	return *Instance_;
}

XServiceCentor::XServiceCentor()
    :isLockCreate_(false)
{
}

XServiceCentor::~XServiceCentor()
{
    for (auto& iter : mapServices_)
    {
        delete iter.second;
    }
    mapServices_.clear();
}

void XServiceCentor::start()
{
    assert(!isLockCreate_);
    isLockCreate_ = true;
}

XService* XServiceCentor::createService(XRobot* robot)
{
    if (!robot)
    {
        assert(false);
        return nullptr;
    }
    if (isLockCreate_)
    {
        XERROR("robotId=%d", robot->robotId_);
        assert(false);
        return nullptr;
    }
    auto findIter = mapServices_.find(robot->robotId_);
    if (findIter != mapServices_.end())
    {
        assert(false);
        return nullptr;
    }
    auto& serviceName = robot->serviceName();
    XService* service = NULL;
    auto iter = mapClassCreate_.find(serviceName);
    if (iter == mapClassCreate_.end())
    {
        XERROR("Service is not exist![%s]", serviceName.data());
        assert(false);
        return 0;
    }
    service = iter->second(robot);
    mapServices_[robot->robotId_] = service;
    return service;
}
