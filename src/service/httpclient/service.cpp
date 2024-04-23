#include "service.h"
#include <cassert>
#include <iostream>
#include "utils/event.h"

namespace HttpClient
{

Service::Service(XRobot* robot)
	:XServiceHttpClient(robot)
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
	this->sleep(3000);

	auto request = createHttp();
	request->setUrl("http://127.0.0.1:8080/index.html");
	XHttpResponse* response = httpGet(request);
	if (!response)
	{
		XINFO("httpGet url failed. %s", request->url_.data());
		return;
	}
	XDEBUG("==>> code:%d", response->code_);
	XDEBUG("==>> head[%d]:%s", response->head_.size(), response->head_.data());
	XDEBUG("==>> body[%d]:%s", response->body_.size(), response->body_.data());
	XINFO("");
}

void Service::onStop()
{
	XDEBUG("===>>");
}



}

