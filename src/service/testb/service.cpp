#include "service.h"
#include "utils/event.h"
#include "handle.h"

namespace TestB
{

Service::Service(XRobot* robot)
	:XService(robot)
{
	assert(robot);
	BIND_EVENT(EventPing);
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
}

void Service::onStop()
{
	XDEBUG("===>>");
}

}