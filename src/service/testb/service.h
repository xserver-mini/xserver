#pragma once

#include "core/service/service.h"

namespace TestB
{

class Service : public XService
{
public:
	Service(XRobot* robot);
	virtual ~Service();
	static 	Service* New(XRobot* robot);

	virtual void onInit();
	virtual void onStart();
	virtual void onStop();

};

}