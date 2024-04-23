#pragma once

#include "service.h"
#include "utils/event.h"

namespace TestA
{

class Handle
{
public:
	static void OnEventPong(Service& service, const EventPong& event)
	{
		XINFO("=====>>EventPong");
	}
};

};