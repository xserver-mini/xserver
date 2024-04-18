#pragma once

#include "core/socket/servicesocket.h"

namespace TcpListen
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
protected:
	virtual void onSocketOpen(int fd);
	virtual void onSocketAccept(int fd, int afd, const std::string& ip, int port);
	virtual void onSocketClose(int fd, const char* info);
	virtual void onSocketError(int fd, const char* info);

};

}

namespace TcpAccept
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

protected:
	virtual void onSocketAccept(int fd, int afd, const std::string& ip, int port);
	virtual void onSocketOpen(int fd);
	virtual void onSocketData(int fd, const char* data, size_t size);
	virtual void onSocketClose(int fd, const char* info);
	virtual void onSocketError(int fd, const char* info);
};

}