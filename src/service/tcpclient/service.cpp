#include "service.h"
#include <cassert>
#include <iostream>
#include "utils/event.h"

namespace TcpClient
{

Service::Service(XRobot* robot)
	:XServiceSocket(robot)
{
	assert(robot);
}

Service::~Service()
{
}

Service* Service::New(XRobot* robot)
{
	return new Service(robot);
}

void Service::onInit()
{
	XDEBUG("===>>");
}

void Service::onStart()
{
	XDEBUG("===>>");
	this->sleep(1000);
	focusFd_ = connectSocket("127.0.0.1", 51888);
}

void Service::onStop()
{
	XDEBUG("===>>");
}

void Service::onSocketOpen(int fd)
{
	XDEBUG("===>>robotId=%d, fd=%d", robotId_, fd);
	std::string buffer = "hello world123456789";
	sendSocket(fd, buffer.data(), (int)buffer.size());
}

void Service::onSocketData(int fd, const char* data, size_t size)
{
	XDEBUG("%s[%d]fd=%d,size=%d, data=%s\n", serviceName_.data(), robotId_, fd, size, data);
}

void Service::onSocketClose(int fd, const char* info)
{
	XDEBUG("%s[%d]fd=%d,%s\n", serviceName_.data(), robotId_, fd, info);
}

void Service::onSocketError(int fd, const char* info)
{
	XDEBUG("%s[%d]fd=%d,%s\n", serviceName_.data(), robotId_, fd, info);
}

}

