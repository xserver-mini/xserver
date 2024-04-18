#include "service.h"
#include "handle.h"
#include "service/udpport/event.h"
#include "utils/events.h"

namespace UdpDevice
{

Service::Service(XRobot* robot)
	:XService(robot)
{
	assert(robot);
	BIND_EVENT(EventUdpState);
	BIND_EVENT(EventUdpData);
	BIND_EVENT(EventUdpError);
}

Service::~Service()
{
}

Service* Service::New(XRobot* robot)
{
	return new Service(robot);
}

void Service::onInit()
{
	XDEBUG("===>>");
}

void Service::onStart()
{
	XDEBUG("===>>");
	//connectUdp("127.0.0.1", 51888);
	//auto test = XConfig::GetInstance().getValueInt("default", "test", 0);
	//if (!test)
	//{
	//	return;
	//}
	//startEventTimer(0, 3000, true, 16888);
}

void Service::onStop()
{
	XDEBUG("===>>");
}

void Service::onEventTimer(const XEventTimerMsg& event)
{
	XASSERT(event.custom_ == 16888);
	for (auto& iter : mapDevices_)
	{
		auto& device = iter.second;
		XINFO("[%s:%d]sendTotal:%f MB, sendPer:%f KB, receiveTotal:%f MB, receivePer:%f KB", device.ip_.data(), device.port_, 
			device.sendSizeTotal_ * 1.0 / 1024 / 1024, 
			device.sendSizePer_ * 1.0 / 1024 / 3,
			device.receiveSizeTotal_ * 1.0 / 1024 / 1024,
			device.receiveSizePer_ * 1.0 / 1024 / 3);
		device.sendSizePer_ = 0;
		device.receiveSizePer_ = 0;
	}
}

bool Service::connectUdp(const std::string& ip, int port)
{
	XUdpAddress address;
	XServiceSocket::IpPortToUdpAddress(ip, port, address);
	auto device = getDevice(address, true);
	if (!device)
	{
		XASSERT(false);
		return false;
	}
	XASSERT(device->ip_ == ip);
	XASSERT(device->port_ == port);
	XASSERT(device->service_ == this);
	if (device->isConnect_)
	{
		return true;
	}
	device->connect();
	return false;
}

Device* Service::getDevice(const XUdpAddress& address, bool isCreate)
{
	auto iter = mapDevices_.find(address);
	if (iter == mapDevices_.end())
	{
		if (isCreate)
		{
			auto& device = mapDevices_[address];
			device.service_ = this;
			device.setAddress(address);
			return &device;
		}
		return 0;
	}
	auto& device = iter->second;
	XASSERT(std::memcmp(&device.udpAddress_, &address, sizeof(address)) == 0);
	XASSERT(device.service_ == this);
	return &device;
}

void Service::enverDevice(const XUdpAddress& address, int64_t uuid)
{
	auto device = getDevice(address, true);
	if (!device)
	{
		XASSERT(false);
		return;
	}
	device->onOpen(uuid);
}

}