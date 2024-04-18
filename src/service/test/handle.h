#pragma once

#include "service.h"
#include "utils/event.h"

namespace Test
{

class Handle
{
public:
	static void OnEventPingPong(Service& service, const EventPingPong& event)
	{
		XINFO("=====>>EventPingPong");
	}
};

};