#pragma once

#include "utils/events.h"


class Ctrl
{
public:
	Ctrl();
	~Ctrl();

	void start();
	bool sendEvent(XEvent* event, int dstId);

	static Ctrl& GetInstance();
protected:
	static Ctrl* Instance_;
};

