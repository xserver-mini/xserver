/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/


#include "socketserver.h"
#include "socketcentor.h"
#include "eventsocket.h"
#include "../robot/robotcentor.h"

//XSocketServer
XSocketServer::XSocketServer(int serverId, const std::string& listenName, const std::string& acceptName, int threadNum)
    :serverId_(serverId),
    listenName_(listenName),
    acceptName_(acceptName),
    listenRobot_(nullptr),
    threadNum_(threadNum <= 0 ? 1 : threadNum),
    minRobotId_(serverId * XSocketCentor::EMinSocketRobotId),
    maxRobotId_(serverId * XSocketCentor::EMinSocketRobotId + 0x10000)
{
    XASSERT(serverId < XSocketCentor::EMaxServerNum);
    acceptRobots_.fill(0);
}

XSocketServer::~XSocketServer()
{
}

void XSocketServer::start()
{
    if (!vectEngine_.empty())
    {
        assert(false);
        return;
    }
    XEngine* engine = nullptr;
    for (size_t i = 0; i < threadNum_; i++)
    {
        engine = XEngineCentor::GetInstance().createEngine();
        if (!engine)
        {
            assert(false);
            continue;
        }
        vectEngine_.push_back(engine);
    }
    if (vectEngine_.empty())
    {
        assert(false);
        return;
    }
    int robotId = 0;
    XRobot* robot = nullptr;
    auto& robotCentor = XRobotCentor::GetInstance();
    for (int i = 0; i < acceptRobots_.size(); i++)
    {
        robotId = minRobotId_ + i;
        engine  = vectEngine_[i % vectEngine_.size()];
        robot = robotCentor.createRobot<XRobot>(engine, robotId, acceptName_);
        if (!robot)
        {
            assert(false);
            continue;
        }
        robot->server_ = this;
        acceptRobots_[i] = robot;
    }
    engine = XEngineCentor::GetInstance().createEngine();
    if (!engine)
    {
        assert(false);
        return;
    }
    vectEngine_.push_back(engine);
    robotId = minRobotId_ + (int)acceptRobots_.size();
    assert(robotId == maxRobotId_);
    listenRobot_ = robotCentor.createRobot<XRobot>(engine, robotId, listenName_);
    listenRobot_->server_ = this;
}

bool XSocketServer::hasRobotId(int robotId)
{
    return robotId >= minRobotId_ && robotId <= maxRobotId_;
}

int XSocketServer::getRobotId(int port) const
{
    if (port < 0 || port >= acceptRobots_.size())
    {
        XASSERT(false);
        return -1;
    }
    auto robot = acceptRobots_[port];
    if (!robot)
    {
        XASSERT(false);
        return -1;
    }
    return robot->robotId_;
}

bool XSocketServer::onEventSocket(XEventSocket* event)
{
    if (!event)
    {
        XASSERT(false);
        return false;
    }
    int robotId = event->dstId_;
    if (maxRobotId_ == robotId)
    {
        return listenRobot_->sendEvent(event);
    }
    auto idx = robotId - minRobotId_;
    if (idx < 0 || idx >= acceptRobots_.size())
    {
        delete event;
        XASSERT(false);
        return false;
    }
    auto robot = acceptRobots_[idx];
    if (!robot)
    {
        delete event;
        XASSERT(false);
        return false;
    }
    return robot->sendEvent(event);
}
