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
	bool ret = httpGet(request, [this](XHttpRequest& req, XHttpResponse& resp) {
		XDEBUG("==>>1 code:%d", resp.code_);
		if (!resp.isFinish_)
		{
			this->wakeUp();
			return;
		}
		XDEBUG("==>>1 head[%d]:%s", resp.head_.size(), resp.head_.data());
		XDEBUG("==>>1 body[%d]:%s", resp.body_.size(), resp.body_.data());
		this->wakeUp();
	});
	if (!ret)
	{
		XINFO("sendHttp url failed. %s", request->url_.data());
		return;
	}

	//wakeUp
	this->await();

	XHttpResponse* resp = request->resp();
	XDEBUG("==>>2 code:%d", resp->code_);
	XDEBUG("==>>2 head[%d]:%s", resp->head_.size(), resp->head_.data());
	XDEBUG("==>>2 body[%d]:%s", resp->body_.size(), resp->body_.data());
	XINFO("");
}

void Service::onStop()
{
	XDEBUG("===>>");
}



}

