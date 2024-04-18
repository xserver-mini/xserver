#pragma once

#include <string>
#include <cstdint>
#include "core/socket/servicesocket.h"
#include "data.h"
#include "service/udpport/event.h"

namespace UdpDevice
{

class Service;
class Device
{
public:
	Device();
	~Device();
	void clear();

	void setAddress(const XUdpAddress& address);
	void connect();

	void openCtrl();

	void sendData(uint16_t msgId, const char* data, size_t size);
	void onData(const EventUdpData& event);

	void onOpen(int64_t uuid);
	void onClose();
	void onError(const char* data);

	void sendTest(int count);
	void onTest(const MsgTest& msgTest);
	int testCount_;
	int64_t testStartTime_;

	//
	uint16_t reqId_;
	int64_t uuid_;
	XUdpAddress udpAddress_;
public:
	int64_t sendSizeTotal_;
	int64_t sendSizePer_;
	int64_t receiveSizeTotal_;
	int64_t receiveSizePer_;


	uint16_t cacheReqId_;
	std::vector<std::string*> vectCache_;

	int port_;
	std::string ip_;
	bool isConnect_;
	EDeviceStatus status_;

	Service* service_;
};

}
