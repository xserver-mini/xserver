#include "service.h"
#include "utils/event.h"
#include "handle.h"

namespace TestA
{

Service::Service(XRobot* robot)
	:XService(robot)
{
	assert(robot);
	BIND_EVENT(EventPong);
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

	{
		auto eventPing = new EventPing;
		auto retEvent = callEvent(eventPing, ERobotIDTestB);
		if (retEvent)
		{
			XASSERT(retEvent->getEventId() == EventPong::EEventID);
			const EventPong* eventPong = dynamic_cast<const EventPong*>(retEvent.get());
			XASSERT(eventPong);
		}
		else
		{
			XASSERT(false);
		}
	}

	auto event2 = new EventPing;
	sendEvent(event2, ERobotIDTestB);
}

void Service::onStop()
{
	XDEBUG("===>>");
}

}