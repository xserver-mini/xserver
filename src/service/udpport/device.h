#pragma once

#include "core/socket/servicesocket.h"
#include <map>
#include <list>
#include "data.h"

//#define UDP_MSG_DEBUG

struct EventUdpState;

namespace UdpPort
{

class Service;

enum EConnectDetect
{
	EConnectDetectPing = 1,
	EConnectDetectPong = 2
};

class Device
{
public:
	Device();
	~Device();

	void clearList();
	void removeSendList(int32_t receivedx);

	void onOpen(EConnectDetect eDetect, int64_t uuid, const std::string& ip, int port);
	void onClose();
	void sendPackage(UdpPackage* package);
	void sendData(uint16_t msgId, const char* data, uint16_t size);
	void onData(const char* data, uint16_t size);

	bool sendEvent(EventUdpState* event);
	void sendSocketData(const char* data, uint16_t size);
	void onSocketData(const char* data, uint16_t size);
		
	void sendStart();
	void onStart(const UdpPackStart& packStart);
	void sendHeartBeat();
	void onHeartBeat(const UdpPackHeart& packHeart);
	void sendRepeat();
	void onRepeat(const UdpPackRepeat& packRepeat);

	void sendTest(int count);
	void onTest(const UdpPackTest& packTest);
	int testCount_;
	int64_t testStartTime_;

	static const char* GetUdpPackName(int packId);
public:
	int64_t uuid_ = 0;
	int64_t nativeUuid_ = 0;
	int port_ = 0;
	std::string ip_;
	XUdpAddress udpAddress_;
	int connectDetect_ = 0;

	bool isConnect_;
	int heartBeat_;
	int beatCount_ = 0;
	int64_t lastTime_ = 0;
	Service* service_ = 0;

	UdpPackData tmpPackData_;

	int32_t sendNextIdx_;
	int32_t receiveIdx_;

	std::list<UdpPackage*> sendList_;
	std::list<UdpPackage*> recevieList_;

	//tmp
	std::unordered_set<int32_t> tmpSetSidx_;
};

}
