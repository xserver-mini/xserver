#include "service.h"
#include <cassert>
#include <iostream>
#include "utils/event.h"
#include "handle.h"

namespace Test
{

Service::Service(XRobot* robot)
	:XService(robot)
{
	assert(robot);
	BIND_EVENT(EventPingPong);
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

	auto event = new EventPingPong;
	sendEvent(event, ERobotIDTest);
	//or 
	//sendEvent(event, robotId_);
}

void Service::onStop()
{
	XDEBUG("===>>");
}

}