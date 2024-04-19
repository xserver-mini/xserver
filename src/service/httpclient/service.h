#pragma once

#include "core/http/serviceclient.h"

namespace HttpClient
{

class Service : public XServiceHttpClient
{
public:
	Service(XRobot* robot);
	virtual ~Service();
	static 	Service* New(XRobot* robot);

	virtual void onInit();
	virtual void onStart();
	virtual void onStop();

protected:
};

}
