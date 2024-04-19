#include "service.h"
#include <cassert>
#include <iostream>
#include "utils/event.h"

namespace TcpListen
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
	focusFd_ = listenSocket("127.0.0.1", 51888);
}

void Service::onStop()
{
	XDEBUG("===>>");
}

void Service::onSocketOpen(int fd)
{
	XDEBUG("===>>robotId=%d, fd=%d", robotId_, fd);
}

void Service::onSocketAccept(int fd, int afd, const std::string& ip, int port)
{
	XDEBUG("%s[%d]fd=%d, afd=%d, %s:%d\n", serviceName_.data(), robotId_, fd, afd, ip.data(), port);
	dispatchSocketAccept(afd, ip.data(), port);
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

namespace TcpAccept
{

Service::Service(XRobot* robot)
	:XServiceSocket(robot)
{
	assert(robot);
	//BIND_EVENT(EventTimerEvent);
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
	//std::cout << "TcpAccept::Service::onInit ===>>" << std::endl;
}

void Service::onStart()
{
	//std::cout << "TcpAccept::Service::onStart ===>>" << std::endl;
}

void Service::onStop()
{
	//std::cout << "TcpAccept::Service::onStop ===<<" << std::endl;
}

void Service::onSocketAccept(int fd, int afd, const std::string& ip, int port)
{
	XASSERT(fd == afd);
	XDEBUG("%s[%d]fd=%d, afd=%d, %s:%d\n", serviceName_.data(), robotId_, fd, afd, ip.data(), port);
	startSocket(fd);
}

void Service::onSocketOpen(int fd)
{
	XDEBUG("===>>robotId=%d, fd=%d", robotId_, fd);
}

void Service::onSocketData(int fd, const char* data, size_t size)
{
	XDEBUG("%s[%d]fd=%d, size=%d, data=%s\n", serviceName_.data(), robotId_, fd, size, data);
	std::string buffer = "[from TCP Server]";
	buffer.append(data, size);
	sendSocket(fd, buffer.data(), (int)buffer.size());
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