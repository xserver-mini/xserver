/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/


#pragma once

#include "eventsocket.h"
#include "../service/service.h"
#include "../open/opensocket.h"

enum EXSocketEvent
{
	EXSocketData,
	EXSocketClose,
	EXSocketOpen,
	EXSocketAccept,
	EXSocketError,
	EXSocketUdp,
	EXSocketWarning,
};

#ifdef __cplusplus
extern "C" {
#endif

enum EXUdpAddress
{
	EXUdpAddressUDP = 1,
	EXUdpAddressUDPPv6 = 2,
};
//1 + 2 + 4 + 1
//1 + 2 + 16 + 1
struct XUdpAddress
{
	uint8_t type;
	uint16_t port;
	char address[16];
};

#ifdef __cplusplus
}
#endif

struct XUnorderedMapHash
{
	size_t operator()(const XUdpAddress& a) const;
};

struct XUnorderedMapEqual
{
	bool operator()(const XUdpAddress& a1, const XUdpAddress& a2) const;
};

class XSocketServer;
class XServiceSocket : public XService
{
public:
	XServiceSocket(XRobot* robot);
	virtual ~XServiceSocket();
	static 	XServiceSocket* New(XRobot* robot);

	virtual void onInit();
	virtual void onStart();
	virtual void onStop();
	int listenSocket(const std::string& ip, int port);
	int connectSocket(const std::string& ip, int port);
	int sendSocket(int fd, const char* data, size_t size);
	void startSocket(int fd);
	void closeSocket(int fd);
	void dispatchSocketAccept(int fd, const std::string& ip, int port);
	const XSocketServer* getSocketServer();

	int udpSocket(const std::string& ip = "", int port = 0);
	int connectUdpSocket(int fd, const std::string& ip, int port);
	int sendUdpSocket(int fd, const char* data, uint16_t size, const XUdpAddress& address);
	int sendUdpSocket(int fd, const char* data, uint16_t size, const std::string& ip, int port);

	static int UdpAddressToIpPort(const XUdpAddress& address, std::string& ip, int& port);
	static int IpPortToUdpAddress(const std::string& ip, const int& port, XUdpAddress& address);

	virtual void onSocketOpen(int fd);
	virtual void onSocketAccept(int fd, int afd, const std::string& ip, int port);
	virtual void onSocketData(int fd, const char* data, size_t size);
	virtual void onSocketWarning(int fd, const char* info);
	virtual void onSocketClose(int fd, const char* info);
	virtual void onSocketError(int fd, const char* info);
	virtual void onSocketUdp(int fd, const char* data, uint16_t size, const XUdpAddress& address);

	int focusFd_;
protected:
	virtual void onEvent(const XEvent& event);
	virtual void onEventSocket(const XEventSocket& event);

};