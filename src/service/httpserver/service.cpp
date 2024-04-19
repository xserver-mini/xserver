#include "service.h"
#include "utils/event.h"

namespace HttpListen
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
	focusFd_ = listenSocket("127.0.0.1", 8080);
	XDEBUG("===>>127.0.0.1:8080");
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

namespace HttpAccept
{

Service::Service(XRobot* robot)
	:XServiceHttpServer(robot)
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
#ifdef USE_OPEN_SSL
	//openSSL("www.xx.com.key", "www.xx.com.crt");
#endif
}

void Service::onStart()
{
}

void Service::onStop()
{
}

bool Service::onHttp(XHttpRequest& req, XHttpResponse& rep)
{
	return handle_.onCallBack(req, rep);;
}

}