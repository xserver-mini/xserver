/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/


#define _CRT_SECURE_NO_WARNINGS

#include "socketcentor.h"
#include "eventsocket.h"
#include "socketserver.h"
#include "../open/opensocket.h"
#include "../robot/robotcentor.h"

static void SocketFunc(const open::OpenSocketMsg* msg)
{
    if (!msg) return;
    if (msg->uid_ < 0)
    {
        delete msg; 
        return;
    }
    //switch (msg->type_)
    //{
    //case OpenSocket::ESocketError:
    //    XERROR("ESocketError robotId=%d, fd=%d. error(%d): [%s]", msg->uid_, msg->fd_, errno, strerror(errno));
    //    break;
    //break;
    //case OpenSocket::ESocketWarning:
    //    XERROR("ESocketWarning robotId=%d, fd=%d. error(%d): [%s]", msg->uid_, msg->fd_, errno, strerror(errno));
    //    break;
    //case OpenSocket::ESocketData:
    //case OpenSocket::ESocketUdp:
    //case OpenSocket::ESocketClose:
    //case OpenSocket::ESocketOpen:
    //case OpenSocket::ESocketAccept:
    //default:
    //    break;
    //}

    auto event = new XEventSocketMsg;
    event->srcId_ = EXRobotIDSocket;
    event->dstId_ = (int)msg->uid_;
    event->socketMsg_ = msg;
    XSocketCentor::GetInstance().onEventSocket(event);
}

XSocketCentor* XSocketCentor::Instance_ = nullptr;

XSocketCentor& XSocketCentor::GetInstance()
{
	if (!Instance_)
	{
		Instance_ = new XSocketCentor;
	}
	return *Instance_;
}

XSocketCentor::XSocketCentor()
    :isLockCreate_(false)
{
}

XSocketCentor::~XSocketCentor()
{
    for (auto& server : vectSevers_)
        delete server;
    
    vectSevers_.clear();
}

void XSocketCentor::start()
{
    assert(!isLockCreate_);
    if (isLockCreate_) return;
    isLockCreate_ = true;
    open::OpenSocket::Start(SocketFunc);
    for (auto& server : vectSevers_)
    {
        assert(server);
        server->start();
    }
}

bool XSocketCentor::isHereRobot(int robotId)
{
    return robotId >= EMinSocketRobotId;
}

bool XSocketCentor::onEventSocket(XEventSocket* event)
{
    if (!event)
    {
        XASSERT(false);
        return false;
    }
    int robotId = event->dstId_;
    if (robotId >= EMinSocketRobotId)
    {
        auto serverId = (robotId - robotId % EMinSocketRobotId) / EMinSocketRobotId;
        if (serverId <= 0 || serverId > vectSevers_.size())
        {
            XERROR("invalid robotId = %d", robotId);
            delete event;
            XASSERT(false);
            return false;
        }
        auto server = vectSevers_[serverId - 1];
        if (!server)
        {
            delete event;
            XASSERT(false);
            return false;
        }
        XASSERT(server->serverId_ == serverId);
        return server->onEventSocket(event);
    }
    bool ret = XRobotCentor::GetInstance().sendEventInter(event);
    if (!ret)
    {
        XERROR("dispatch faild robotId = %d", robotId);
        XASSERT(false);
        return false;
    }
    return true;
}

int XSocketCentor::createServer(const std::string& listenName, const std::string& acceptName, int engineNum, int robotNum)
{
    if (vectSevers_.size() >= EMaxServerNum)
    {
        XASSERT(false);
        return -1;
    }
    int serverId = (int)vectSevers_.size() + 1;
    auto server = new XSocketServer(serverId, listenName, acceptName, engineNum, robotNum);
    vectSevers_.push_back(server);
    return serverId;
}
