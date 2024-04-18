#pragma once

#include "core/service/service.h"
#include "device.h"
#include "core/socket/servicesocket.h"
#include "data.h"

namespace UdpDevice
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
	bool connectUdp(const std::string& ip, int port);

	virtual void onEventTimer(const XEventTimerMsg& event);

	Device* getDevice(const XUdpAddress& address, bool isCreate = false);
	void enverDevice(const XUdpAddress& address, int64_t uuid);

protected:
	std::unordered_map<XUdpAddress, Device, XUnorderedMapHash, XUnorderedMapEqual> mapDevices_;
};

}