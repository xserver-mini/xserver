#pragma once

#include "core/socket/servicesocket.h"
#include "device.h"

namespace UdpPort
{

class Service : public XServiceSocket
{
	struct IpData
	{
		std::string ip_;
		std::string template_;
	};
public:
	Service(XRobot* robot);
	virtual ~Service();
	static 	Service* New(XRobot* robot);

	void bindAddr();
	void loadLocalIps();
	void startPings();
	bool sendPings();
	int sendPing(const std::string& ip, int port, bool isTest = false);
	int sendPing(const XUdpAddress& address, bool isTest = false);
	int sendPong(const XUdpAddress& address);
	void sendHeartBeat();

	virtual void onInit();
	virtual void onStart();
	virtual void onStop();

	virtual void onEventTimer(const XEventTimerMsg& event);
	virtual void onSocketOpen(int fd);
	virtual void onSocketUdp(int fd, const char* data, uint16_t size, const XUdpAddress& address);
	virtual void onSocketWarning(int fd, const char* info);
	virtual void onSocketClose(int fd, const char* info);

	void setState(int state);
	Device* getDevice(const XUdpAddress& address);
	int64_t uuid_;
protected:
	void onPing(const XUdpAddress& address, const UdpPackPing& packPing);
	void onPong(const XUdpAddress& address, const UdpPackPong& packPong);
	void onHeart(const XUdpAddress& address, const UdpPackHeart& packHeart);
	void onTest(const XUdpAddress& address, const UdpPackTest& packTest);
	void onData(const XUdpAddress& address, const char* data, uint16_t size);

	void saveDevices();
	void pingCacheDevices();

	int tmpPort_;
	std::string tmpIp_;
	std::string tmpKey_;

	int state_;
	int detectPort_;
	int bindPort_;
	int heartBeat_;
	std::string nativeIp_;
	std::queue<std::string> queueDetectIp_;
	std::unordered_map<std::string, IpData> mapDetectIpDatas_;
	std::unordered_map<XUdpAddress, Device, XUnorderedMapHash, XUnorderedMapEqual> mapDevices_;
};

}
