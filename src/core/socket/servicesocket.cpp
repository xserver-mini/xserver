/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/

#define _CRT_SECURE_NO_WARNINGS

#include "servicesocket.h"
#include "socketserver.h"
#include "../robot/robot.h"
#include "../open/opensocket.h"
using namespace open;

size_t XUnorderedMapHash::operator()(const XUdpAddress& a) const
{
	if (a.type == EXUdpAddressUDP)
	{
		size_t h = *((uint32_t*)a.address);
		h = (h << 16) | a.port;
		return h;
	}
	std::string key(((const char*)&a + 1), sizeof(a) - 2);
	return std::hash<std::string>()(key);
}

bool XUnorderedMapEqual::operator()(const XUdpAddress& a1, const XUdpAddress& a2) const
{
	if (a1.type != a2.type)
	{
		return false;
	}
	if (a1.type == EXUdpAddressUDP)
	{
		return a1.port == a2.port && (*((uint32_t*)a1.address) == *((uint32_t*)a2.address));
	}
	return std::memcmp(&a1, &a2, sizeof(a1)) == 0;
}


XServiceSocket::XServiceSocket(XRobot* robot)
	:XService(robot),
	focusFd_(-1)
{
	XASSERT(robot);
}

XServiceSocket::~XServiceSocket()
{
}

XServiceSocket* XServiceSocket::New(XRobot* robot)
{
	return new XServiceSocket(robot);
}

void XServiceSocket::onInit()
{
	XDEBUG("==>>");
}

void XServiceSocket::onStart()
{
	XDEBUG("==>>");
}

void XServiceSocket::onStop()
{
	XDEBUG("==>>");
}

int XServiceSocket::listenSocket(const std::string& ip, int port)
{
	//XDEBUG("robotId=%d, ip=%s, port=%d", robotId_, ip.data(), port);
	if (robotId_ < 0)
	{
		XASSERT(false);
		return -1;
	}
	int fd = open::OpenSocket::Instance().listen((uintptr_t)robotId_, ip, port, 64);
	if (fd < 0)
	{
		XERROR("faild fd = %d, error(%d): %s", fd, errno, strerror(errno));
		XASSERT(false);
		return -1;
	}
	open::OpenSocket::Instance().start((uintptr_t)robotId_, fd);
	return fd;
}

int XServiceSocket::connectSocket(const std::string& ip, int port)
{
	//XDEBUG("robotId=%d, ip=%s, port=%d", robotId_, ip.data(), port);
	if (robotId_ <= 0)
	{
		XASSERT(false);
		return -1;
	}
	int fd = open::OpenSocket::Instance().connect(robotId_, ip, port);
	if (fd < 0)
	{
		XERROR("faild fd = %d, error(%d): %s", fd, errno, strerror(errno));
	}
	return fd;
}

int XServiceSocket::sendSocket(int fd, const char* data, size_t size)
{
	//XDEBUG("robotId=%d, size=%d", robotId_, (int)size);
	int ret = OpenSocket::Instance().send(fd, data, (int)size);
	if (ret < 0)
	{
		XERROR("faild fd = %d, error(%d): %s", fd, errno, strerror(errno));
	}
	return ret;
}

void XServiceSocket::startSocket(int fd)
{
	//XDEBUG("robotId=%d, fd=%d", robotId_, fd);
	if (robotId_ < 0)
	{
		XASSERT(false);
		return;
	}
	open::OpenSocket::Instance().start((uintptr_t)robotId_, fd);
}

void XServiceSocket::closeSocket(int fd)
{
	XDEBUG("robotId=%d, fd=%d", robotId_, fd);
	if (robotId_ <= 0)
	{
		XASSERT(false);
		return;
	}
	OpenSocket::Instance().close(robotId_, fd);
}

void XServiceSocket::dispatchSocketAccept(int fd, const std::string& ip, int port)
{
	if (fd == focusFd_)
	{
		XWARN("%s[%d]fd=%d, notice fd == focusFd_  %s:%d", serviceName_.data(), robotId_, fd, ip.data(), port);
	}
	const XSocketServer* server = getSocketServer();
	if (!server)
	{
		XASSERT(false);
		return;
	}
	int targetRobotId = server->getRobotId(port);
	if (targetRobotId < 0)
	{
		XASSERT(false);
		return;
	}
	auto event = new XEventSocketAccept;
	event->fd_ = fd;
	event->ip_ = ip;
	event->port_ = port;
	sendEvent(event, targetRobotId);
}

const XSocketServer* XServiceSocket::getSocketServer()
{
	if (!robot_) return 0;
	return dynamic_cast<XSocketServer*>((XSocketServer*)robot_->server_);
}

int XServiceSocket::udpSocket(const std::string& ip, int port)
{
	//XDEBUG("robotId=%d, ip=%s, port=%d", robotId_, ip.data(), port);
	if (robotId_ <= 0)
	{
		XASSERT(false);
		return -1;
	}
	int fd = OpenSocket::Instance().udp((uintptr_t)robotId_, ip.empty() ? 0 : ip.data(), port);
	if (fd < 0)
	{
		XERROR("faild fd = %d, error(%d): %s", fd, errno, strerror(errno));
	}
	return fd;
}

int XServiceSocket::connectUdpSocket(int fd, const std::string& ip, int port)
{
	//XDEBUG("robotId=%d, fd=%d, ip=%s, port=%d", robotId_, fd, ip.data(), port);
	int ret = OpenSocket::Instance().udpConnect(fd, ip.data(), port);
	if (ret < 0)
	{
		XERROR("faild fd = %d, error(%d): %s", fd, errno, strerror(errno));
	}
	return ret;
}

int XServiceSocket::sendUdpSocket(int fd, const char* data, uint16_t size, const XUdpAddress& address)
{
	//XDEBUG("robotId=%d, fd=%d, data=%s, size=%d", robotId_, fd, data, size);
	int ret = OpenSocket::Instance().udpSend(fd, (const char*)&address, data, (int)size);
	if (ret != 0)
	{
		XERROR("faild fd = %d, error(%d): %s", fd, errno, strerror(errno));
	}
	return ret;
}

int XServiceSocket::sendUdpSocket(int fd, const char* data, uint16_t size, const std::string& ip, int port)
{
	//XDEBUG("robotId=%d, fd=%d, data=%s, size=%d, ip=%s, port=%d", robotId_, fd, data, size, ip.data(), port);
	uint8_t udp_address[UDP_ADDRESS_SIZE] = {0};
	int ret = OpenSocket::IpPortToUDPAddress(ip.data(), port, udp_address, UDP_ADDRESS_SIZE);
	if (ret < 0)
	{
		XASSERT(false);
		return ret;
	}
	ret = OpenSocket::Instance().udpSend(fd, (const char*)udp_address, data, (int)size);
	if (ret != 0)
	{
		XERROR("faild fd = %d, error(%d): %s", fd, errno, strerror(errno));
	}
	return ret;
}

int XServiceSocket::UdpAddressToIpPort(const XUdpAddress& address, std::string& ip, int& port)
{
	return OpenSocket::UDPAddressToIpPort((const char*)&address, ip, port);
}

int XServiceSocket::IpPortToUdpAddress(const std::string& ip, const int& port, XUdpAddress& address)
{
	//XASSERT(UDP_ADDRESS_SIZE == sizeof(address));
	return OpenSocket::IpPortToUDPAddress(ip.data(), port, (uint8_t*)&address, (int)sizeof(address));
}

void XServiceSocket::onEvent(const XEvent& event)
{
	if (!isRunning())
	{
		XERROR("robot_ is running. %s[%d] eventId=%d", serviceName_.data(), robotId_, event.getEventId());
		assert(false);
		return;
	}
	if (event.eEventType_ == EXEventTypeSocket)
	{
		const XEventSocket* eventSocket = dynamic_cast<const XEventSocket*>(&event);
		if (eventSocket)
		{
			onEventSocket(*eventSocket);
		}
		else
		{
			XASSERT(false);
		}
		return;
	}
	XService::onEvent(event);
}

void XServiceSocket::onEventSocket(const XEventSocket& event)
{
	if (XEventSocketMsg::EEventID == event.getEventId())
	{
		const XEventSocketMsg* tmpEvent = dynamic_cast<const XEventSocketMsg*>(&event);
		if (!tmpEvent)
		{
			XASSERT(false);
			return;
		}
		auto socketMsg = tmpEvent->socketMsg_;
		if (!socketMsg)
		{
			XASSERT(false);
			return;
		}
		const OpenSocketMsg& msg = *socketMsg;
		if (msg.uid_ != robotId_)
		{
			XASSERT(false);
			return;
		}
		//if (focusFd_ >= 0 && focusFd_ != msg.fd_)
		//{
		//	XASSERT(false);
		//	return;
		//}
		switch (msg.type_)
		{
		case OpenSocket::ESocketData:
			onSocketData(msg.fd_, msg.data(), msg.size());
			break;
		case OpenSocket::ESocketUdp:
			XASSERT(msg.size() < 0xffff);
			onSocketUdp(msg.fd_, msg.data(), (uint16_t)msg.size(), *(XUdpAddress*)msg.option_);
			break;
		case OpenSocket::ESocketClose:
			onSocketClose(msg.fd_, msg.data());
			break;
		case OpenSocket::ESocketError:
			onSocketError(msg.fd_, msg.data());
			break;
		case OpenSocket::ESocketOpen:
			onSocketOpen(msg.fd_);
			break;
		case OpenSocket::ESocketAccept:
		{
			const std::string addr = msg.data();
			size_t idx = addr.find(":");
			if (idx != std::string::npos)
			{
				std::string ip = addr.substr(0, idx);
				int port = atoi(addr.data() + idx + 1);
				onSocketAccept(msg.fd_, msg.ud_, ip.data(), port);
			}
			else
			{
				XASSERT(false);
				closeSocket(msg.fd_);
			}
		}
		break;
		case OpenSocket::ESocketWarning:
			onSocketWarning(msg.fd_, msg.data());
			break;
		default:
			XASSERT(false);
			break;
		}
	}
	else if (XEventSocketAccept::EEventID == event.getEventId())
	{
		const XEventSocketAccept* tmpEvent = dynamic_cast<const XEventSocketAccept*>(&event);
		if (!tmpEvent)
		{
			XASSERT(false);
			return;
		}
		onSocketAccept(tmpEvent->fd_, tmpEvent->fd_, tmpEvent->ip_.data(), tmpEvent->port_);
	}
}

void XServiceSocket::onSocketOpen(int fd)
{
	XASSERT(false);
}

void XServiceSocket::onSocketAccept(int fd, int afd, const std::string& ip, int port)
{
	XDEBUG("%s[%d]fd=%d, afd=%d, %s:%d\n", serviceName_.data(), robotId_, fd, afd, ip.data(), port);
	assert(false);
}

void XServiceSocket::onSocketData(int fd, const char* data, size_t size)
{
	XDEBUG("%s[%d]fd=%d,size=%s\n", serviceName_.data(), robotId_, fd, size);
	assert(false);
}

void XServiceSocket::onSocketWarning(int fd, const char* info)
{
	XDEBUG("%s[%d]fd=%d,%s\n", serviceName_.data(), robotId_, fd, info);
}

void XServiceSocket::onSocketClose(int fd, const char* info)
{
	XDEBUG("%s[%d]fd=%d,%s\n", serviceName_.data(), robotId_, fd, info);
	assert(false);
}

void XServiceSocket::onSocketError(int fd, const char* info)
{
	XDEBUG("%s[%d]fd=%d,%s\n", serviceName_.data(), robotId_, fd, info);
}

void XServiceSocket::onSocketUdp(int fd, const char* data, uint16_t size, const XUdpAddress& address)
{
	assert(false);
}
