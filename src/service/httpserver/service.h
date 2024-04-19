#pragma once

#include "core/http/serviceserver.h"
#include "handle.h"

namespace HttpListen
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

namespace HttpAccept
{

class Service : public XServiceHttpServer 
{
public:
	Service(XRobot* robot);
	virtual ~Service();
	static 	Service* New(XRobot* robot);

	virtual void onInit();
	virtual void onStart();
	virtual void onStop();
	virtual bool onHttp(XHttpRequest& req, XHttpResponse& rep);

protected:
	Handle handle_;
};

}