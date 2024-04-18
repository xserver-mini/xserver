#pragma once

#include "core/socket/servicesocket.h"

namespace TcpClient
{

class Service : public XServiceSocket
{
public:
	Service(XRobot* robot);
	virtual ~Service();
	static 	Service* New(XRobot* robot);

	virtual void onInit();
	virtual void onStart();
	virtual void onStop();

	virtual void onSocketOpen(int fd);
	virtual void onSocketData(int fd, const char* data, size_t size);
	virtual void onSocketClose(int fd, const char* info);
	virtual void onSocketError(int fd, const char* info);
protected:
};

}
