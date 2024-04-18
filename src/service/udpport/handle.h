#pragma once

#include "service.h"
#include "event.h"
#include "device.h"

namespace UdpPort
{

class Handle
{
public:
	static void OnEventUdpData(Service& service, const EventUdpData& event)
	{
		if (event.estate_ != EUdpStateSend)
		{
			XASSERT(false);
			return;
		}
		//XINFO("=========================================>>EUdpStateSend");
		auto device = service.getDevice(event.address_);
		if (device)
		{
			auto buffer = event.getBuffer();
			XASSERT(buffer->size() < 0xffff);
			device->sendData(event.msgId_, buffer->data(), (uint16_t)buffer->size());
			return;
		}
		auto sevent = new EventUdpError;
		sevent->estate_ = EUdpStateError;
		sevent->address_ = event.address_;
		sevent->buffer_ = "disconnect";
		service.returnEvent(sevent);
	}

	static void OnEventUdpPackage(Service& service, const EventUdpPackage& event)
	{
		if (event.estate_ != EUdpStateSend)
		{
			XASSERT(false);
			return;
		}
		//XINFO("=========================================>>EUdpStateSend");
		auto device = service.getDevice(event.address_);
		if (device && device->isConnect_)
		{
			auto pevent = (EventUdpPackage*)&event;
			device->sendPackage(pevent->swap());
			return;
		}
		auto sevent = new EventUdpError;
		sevent->estate_ = EUdpStateError;
		sevent->address_ = event.address_;
		sevent->buffer_ = "disconnect";
		service.returnEvent(sevent);
	}

	static void OnEventUdpPackages(Service& service, const EventUdpPackages& event)
	{
		if (event.estate_ != EUdpStateSend)
		{
			XASSERT(false);
			return;
		}
		//XINFO("=========================================>>EUdpStateSend");
		auto device = service.getDevice(event.address_);
		if (device && device->isConnect_)
		{
			for (size_t i = 0; i < event.vect_.size(); i++)
			{
				auto& package = event.vect_[i];
				XASSERT(package->buffer_.size() < 0xffff);
				device->sendPackage(package);
			}
			((EventUdpPackages*)&event)->vect_.clear();
			return;
		}
		auto sevent = new EventUdpError;
		sevent->estate_ = EUdpStateError;
		sevent->address_ = event.address_;
		sevent->buffer_ = "disconnect";
		service.returnEvent(sevent);
	}

	static void OnEventUdpState(Service& service, const EventUdpState& event)
	{
		if (event.estate_ != EUdpStateConnect)
		{
			XASSERT(false);
			return;
		}
		//XINFO("=========================================>>EUdpStateConnect");
		auto device = service.getDevice(event.address_);
		if (device && device->isConnect_)
		{
			auto sevent = new EventUdpState;
			sevent->estate_ = EUdpStateOpen;
			sevent->address_ = event.address_;
			sevent->uuid_ = device->uuid_;
			service.returnEvent(sevent);
			return;
		}
		service.sendPing(event.address_, false);
	}
};

};