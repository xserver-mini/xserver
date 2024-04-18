#pragma once

#include "core/app.h"

class App : public XApp
{
public:
	App();
	virtual ~App();
	void start();
	virtual void onMainThreadEvent(XEvent* event);
protected:
};