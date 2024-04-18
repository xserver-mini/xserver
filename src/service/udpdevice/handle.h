#pragma once

#include "service.h"
#include "service/udpport/event.h"
#include "event.h"

namespace UdpDevice
{

class Handle
{
public:
	static void OnEventUdpState(Service& service, const EventUdpState& event)
	{
		if (event.estate_ == EUdpStateOpen)
		{
			//XINFO("=========================================>>EUdpStateOpen");
			service.enverDevice(event.address_, event.uuid_);
		}
		else if (event.estate_ == EUdpStateClose)
		{
			//XINFO("=========================================>>EUdpStateClose");
			auto device = service.getDevice(event.address_, false);
			if (!device)
			{
				XASSERT(false);
				return;
			}
			device->onClose();
		}
		else
		{
			XASSERT(false);
		}
	}

	static void OnEventUdpData(Service& service, const EventUdpData& event)
	{
		auto device = service.getDevice(event.address_);
		if (!device)
		{
			XASSERT(false);
			return;
		}
		if (event.estate_ == EUdpStateReceive)
		{
			//XINFO("=========================================>>EUdpStateReceive");
			if (!device->isConnect_)
			{
				XINFO("=========================================>>EUdpStateReceive close.drop data");
				return;
			}
			device->onData(event);
		}
		else
		{
			XASSERT(false);
		}
	}

	static void OnEventUdpError(Service& service, const EventUdpError& event)
	{
		XINFO("=============>>EUdpStateError[%s]", event.buffer_.data());
		auto device = service.getDevice(event.address_);
		if (!device)
		{
			XASSERT(false);
			return;
		}
		device->onError(event.buffer_.data());
	}

};

};