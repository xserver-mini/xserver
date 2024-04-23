#pragma once

#include "service.h"
#include "utils/event.h"

namespace TestB
{

class Handle
{
public:
	static void OnEventPing(Service& service, const EventPing& event)
	{
		XINFO("=====>>EventPing");
		if (event.sessionId_ == 0)
		{
			auto sevent = new EventPong;

			service.sendEvent(sevent, event.srcId_);
			// or 
			// service.returnEvent(sevent);
			return;
		}
		auto sevent = new EventPong;
		service.returnEvent(sevent);
	}
};

};