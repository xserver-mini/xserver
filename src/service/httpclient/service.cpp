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
	bool ret = sendHttp(request, [this](XHttpRequest& req, XHttpResponse& resp) {
		XDEBUG("==>> code:%d", resp.code_);
		if (!resp.isFinish_)
		{
			return;
		}
		XDEBUG("==>> head[%d]:%s", resp.head_.size(), resp.head_.data());
		XDEBUG("==>> body[%d]:%s", resp.body_.size(), resp.body_.data());
	});

	if (!ret)
	{
		XINFO("sendHttp url failed. %s", request->url_.data());
	}
}

void Service::onStop()
{
	XDEBUG("===>>");
}



}

