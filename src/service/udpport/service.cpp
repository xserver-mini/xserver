#include "service.h"
#include "handle.h"
#include "core/utils/cache.h"
#include "core/platform/net.h"

namespace UdpPort
{
enum EServiceState
{
	EServiceStateNone,
	EServiceStateBind,
	EServiceStateTest,
	EServiceStateDetect,
	EServiceStateRun,
};

static const std::string kMLoopIp = "127.0.0.1";

Service::Service(XRobot* robot)
	:XServiceSocket(robot)
{
	assert(robot);
	BIND_EVENT(EventUdpState);
	BIND_EVENT(EventUdpData);
	BIND_EVENT(EventUdpPackage);
	BIND_EVENT(EventUdpPackages);

	focusFd_ = 0;
	state_ = EServiceStateNone;
	detectPort_ = XConfig::GetInstance().getValueInt("UDPPort", "port", 51688);
	bindPort_ = detectPort_;
	heartBeat_ = XConfig::GetInstance().getValueInt("UDPPort", "heartbeat", 10000);

	uuid_ = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	uuid_ = uuid_ * 1000 + std::rand() % 1000;

	setMinCostTime(10);
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
	XASSERT(state_ == EServiceStateNone);
	setState(EServiceStateBind);
}

void Service::onStop()
{
	XDEBUG("===>>");
}

void Service::bindAddr()
{
	if (focusFd_ > 0)
	{
		XINFO("closeSocket focusFd_=%d, bindPort_=%d, detectPort_=%d, uuid_=%lld", focusFd_, bindPort_, detectPort_, uuid_);
		closeSocket(focusFd_);
		focusFd_ = 0;
	}
	focusFd_ = udpSocket("0.0.0.0", bindPort_);
	startSocket(focusFd_);
	//XINFO("focusFd_=%d, bindPort_=%d, detectPort_=%d, uuid_=%lld", focusFd_, bindPort_, detectPort_, uuid_);
}

void Service::loadLocalIps()
{
	mapDetectIpDatas_.clear();
	std::vector<std::string> vectIps;
	XNet::GetLocalIps(vectIps);
	IpData data;
	for (auto& ip : vectIps)
	{
		if (ip.find(":") != std::string::npos)
		{
			continue;
		}
		if (ip == kMLoopIp)
		{
			continue;
		}
		XINFO("loadLocalIps ip:%s", ip.data());
		auto idx = ip.find_last_of(".");
		if (idx == std::string::npos)
		{
			XASSERT(false);
			continue;
		}
		auto& data = mapDetectIpDatas_[ip];
		data.ip_ = ip;
		data.template_ = ip.substr(0, idx + 1);
	}
}

void Service::startPings()
{
	while (!queueDetectIp_.empty()) queueDetectIp_.pop();
	loadLocalIps();

	std::string ip;
	bool isFistDectect = false;
	std::vector<std::string> vectDelayDetectIp;
	for (auto& iter : mapDetectIpDatas_)
	{
		auto& data = iter.second;
		isFistDectect = nativeIp_ == data.ip_;
		for (int id = 2; id < 255; ++id)
		{
			ip = data.template_ + std::to_string(id);
			if (isFistDectect)
				queueDetectIp_.push(ip);
			else
				vectDelayDetectIp.push_back(ip);
		}
	}
	for (auto& ip : vectDelayDetectIp)
	{
		queueDetectIp_.push(ip);
	}
	XINFO("queueDetectIp.size=%d", queueDetectIp_.size());
}

bool Service::sendPings()
{
	if (queueDetectIp_.empty())
	{
		return false;
	}
	UdpPackPing packPing;
	packPing.header_.type_ = EUdpPackPing;
	packPing.uuid_ = uuid_;
	packPing.isTest_ = false;
	XUdpAddress address = { 0 };
	for (int i = 0; i < 50 && i < queueDetectIp_.size(); ++i)
	{
		auto ip = queueDetectIp_.front();
		queueDetectIp_.pop();

		IpPortToUdpAddress(ip, detectPort_, address);
		UdpAddressToIpPort(address, tmpIp_, tmpPort_);
		XASSERT(ip == tmpIp_);
		XASSERT(detectPort_ == tmpPort_);

		auto iter = mapDevices_.find(address);
		if (iter != mapDevices_.end())
		{
			auto& device = iter->second;
			if (device.isConnect_)
			{
				XINFO("sendPings. Device is connnected.%s:%d", ip.data(), detectPort_);
				continue;
			}
		}
		sendUdpSocket(focusFd_, (const char*)&packPing, sizeof(packPing), address);
	}
	//XINFO("sendPings. queueDetectIp.size=%d", queueDetectIp_.size());
	return true;
}

int Service::sendPing(const std::string& ip, int port, bool isTest)
{
	XINFO("========ping==>>[%s:%d] isTest=%d", ip.data(), port, isTest);
	UdpPackPing packPing;
	packPing.header_.type_ = EUdpPackPing;
	packPing.uuid_ = uuid_;
	packPing.isTest_ = isTest;
	return sendUdpSocket(focusFd_, (const char*)&packPing, sizeof(packPing), ip, port);
}

int Service::sendPing(const XUdpAddress& address, bool isTest)
{
	UdpAddressToIpPort(address, tmpIp_, tmpPort_);
	XINFO("======ping====>>[%s:%d] isTest=%d", tmpIp_.data(), tmpPort_, isTest);

	UdpPackPing packPing;
	packPing.header_.type_ = EUdpPackPing;
	packPing.uuid_ = uuid_;
	packPing.isTest_ = isTest;
	return sendUdpSocket(focusFd_, (const char*)&packPing, sizeof(packPing), address);
}

int Service::sendPong(const XUdpAddress& address)
{
	UdpAddressToIpPort(address, tmpIp_, tmpPort_);
	XINFO("======pong===>>[%s:%d]", tmpIp_.data(), tmpPort_);

	UdpPackPong packPong;
	packPong.header_.type_ = EUdpPackPong;
	packPong.uuid_ = uuid_;
	return sendUdpSocket(focusFd_, (const char*)&packPong, sizeof(packPong), address);
}

void Service::sendHeartBeat()
{
	//1472
	for (auto& iter : mapDevices_)
	{
		auto& device = iter.second;
		device.sendHeartBeat();
	}
}

void Service::setState(int state)
{
	switch (state)
	{
	case EServiceStateBind:
		bindAddr();
		startEventTimer(1000, 1000, true, EServiceStateBind);
		break;
	case EServiceStateTest:
		sendPing(kMLoopIp, bindPort_, true);
		break;
	case EServiceStateDetect:
		startPings();
		pingCacheDevices();
		startEventTimer(0, 100, true, EServiceStateDetect);
		break;
	case EServiceStateRun:
		if (nativeIp_.empty())
		{
			sendPing(kMLoopIp, bindPort_, false);
			if (bindPort_ != detectPort_)
			{
				sendPing(kMLoopIp, detectPort_, false);
			}
		}
		startEventTimer(0, heartBeat_, true, EServiceStateRun);
		break;
	default:
		XASSERT(false);
		break;
	}
	state_ = state;
}

void Service::onEventTimer(const XEventTimerMsg& event)
{
	if (state_ == EServiceStateBind || state_ == EServiceStateTest)
	{
		bindPort_++;
		setState(EServiceStateBind);
		return;
	}
	if (state_ == EServiceStateDetect)
	{
		if (event.custom_ != EServiceStateDetect)
		{
			return;
		}
		if (!sendPings())
		{
			setState(EServiceStateRun);
		}
		return;
	}
	if (state_ != EServiceStateRun)
	{
		XASSERT(false);
		return;
	}
	if (event.custom_ != EServiceStateRun)
	{
		return;
	}
	sendHeartBeat();
}

void Service::saveDevices()
{
	XCache cache(sizeof(XUdpAddress));
	for (auto& iter : mapDevices_)
	{
		auto& device = iter.second;
		if (device.ip_ == kMLoopIp)
		{
			XINFO("============>>skip device[%s:%d] uuid_:%lld", device.ip_.data(), device.port_, device.uuid_);
			continue;
		}
		cache.addCell<XUdpAddress>(device.udpAddress_);
		XINFO("============>>list device[%s:%d] uuid_:%lld", device.ip_.data(), device.port_, device.uuid_);
	}
	cache.save("encryption1.bin");
}

void Service::pingCacheDevices()
{
	XCache cache(sizeof(XUdpAddress));
	cache.load("encryption1.bin");
	XUdpAddress* address = 0;
	for (size_t i = 0; i < cache.size(); i++)
	{
		address = cache.getCell<XUdpAddress>(i);
		if (address)
		{
			UdpAddressToIpPort(*address, tmpIp_, tmpPort_);
			if (tmpIp_ == kMLoopIp)
			{
				XINFO("============>>Skip device[%s:%d]", tmpIp_.data(), tmpPort_);
				continue;
			}
			auto iter = mapDetectIpDatas_.find(tmpIp_);
			if (iter != mapDetectIpDatas_.end())
			{
				if (tmpPort_ == bindPort_)
				{
					XINFO("============>>Skip device[%s:%d]", tmpIp_.data(), tmpPort_);
					continue;
				}
			}

			XINFO("============>>Detect device[%s:%d]", tmpIp_.data(), tmpPort_);
			sendPing(*address, false);
		}
	}
}

void Service::onSocketOpen(int fd)
{
	XASSERT(focusFd_ == fd);
	//XDEBUG("===>>focusFd_=%d, fd=%d, bindPort_=%d, detectPort_=%d, uuid_=%lld", focusFd_, fd, bindPort_, detectPort_, uuid_);
	setState(EServiceStateTest);
}

void Service::onPing(const XUdpAddress& address, const UdpPackPing& packPing)
{
	UdpAddressToIpPort(address, tmpIp_, tmpPort_);
	if (uuid_ == packPing.uuid_)
	{
		if (packPing.isTest_)
		{
			XINFO("[%s:%d]======>>Ping bind port.uuid_=%lld, packPing.uuid_=%lld", tmpIp_.data(), tmpPort_, uuid_, packPing.uuid_);
			setState(EServiceStateDetect);
			return;
		}
		if (tmpIp_ != kMLoopIp)
		{
			auto iter = mapDetectIpDatas_.find(tmpIp_);
			if (iter == mapDetectIpDatas_.end())
			{
				XINFO("[%s:%d]======>>Ping No exist. port.uuid_=%lld, packPing.uuid_=%lld", tmpIp_.data(), tmpPort_, uuid_, packPing.uuid_);
				return;
			}
		}
		if (tmpIp_ != kMLoopIp || nativeIp_.empty())
		{
			nativeIp_ = tmpIp_;
		}
		XINFO("[%s:%d]1======>>Ping find myself. uuid_=%lld, packPing.uuid_=%lld, nativeIp_=%s", 
			tmpIp_.data(), tmpPort_, uuid_, packPing.uuid_, nativeIp_.data());
		return;
	}
	if (packPing.isTest_)
	{
		XINFO(" [%s:%d]======>>Ping other bind port.uuid_=%lld, packPing.uuid_=%lld", tmpIp_.data(), tmpPort_, uuid_, packPing.uuid_);
		return;
	}

	if (tmpIp_ == kMLoopIp)
	{
		if (bindPort_ == tmpPort_)
		{
			XINFO("[%s:%d]21======>>Ping find myself. uuid_=%lld, packPing.uuid_=%lld, nativeIp_=%s",
				tmpIp_.data(), tmpPort_, uuid_, packPing.uuid_, nativeIp_.data());
			return;
		}
		if (!nativeIp_.empty() && nativeIp_ != kMLoopIp)
		{
			XUdpAddress udpAddress = { 0 };
			IpPortToUdpAddress(nativeIp_, tmpPort_, udpAddress);
			auto iter = mapDevices_.find(udpAddress);
			if (iter != mapDevices_.end())
			{
				XWARN("Device repeat. drop [%s:%d], use [%s:%d]uuid_=%lld, packPing.uuid_=%lld", tmpIp_.data(), tmpPort_, nativeIp_.data(), tmpPort_, uuid_, packPing.uuid_);
				return;
			}
		}
	}
	else if (tmpIp_ == nativeIp_)
	{
		if (bindPort_ == tmpPort_)
		{
			XINFO("[%s:%d]22======>>Ping find myself. uuid_=%lld, packPing.uuid_=%lld, nativeIp_=%s",
				tmpIp_.data(), tmpPort_, uuid_, packPing.uuid_, nativeIp_.data());
			return;
		}
		XUdpAddress udpAddress = { 0 };
		IpPortToUdpAddress(kMLoopIp, tmpPort_, udpAddress);
		auto iter = mapDevices_.find(udpAddress);
		if (iter != mapDevices_.end())
		{
			iter->second.onClose();
			mapDevices_.erase(iter);
			XWARN("Device repeat. remove [%s:%d] and [%s:%d]uuid_=%lld, packPing.uuid_=%lld", tmpIp_.data(), tmpPort_, nativeIp_.data(), tmpPort_, uuid_, packPing.uuid_);
		}
	}

	auto& device = mapDevices_[address];
	if (device.uuid_ != packPing.uuid_)
	{
		XINFO("[%s:%d] ======>>Ping find new device. uuid_=%lld, packPing.uuid_=%lld", tmpIp_.data(), tmpPort_, uuid_, packPing.uuid_);
		if (device.uuid_ > 0)
		{
			device.onClose();
		}
		device.service_ = this;
		device.nativeUuid_ = uuid_;
		device.udpAddress_ = address;
		device.heartBeat_ = heartBeat_ / 1000 + 5;
		device.onOpen(EConnectDetectPing, packPing.uuid_, tmpIp_, tmpPort_);
	}
	else
	{
		XINFO("[%s:%d]======>>Ping already find device. uuid_:%lld, packPing.uuid_=%lld", tmpIp_.data(), tmpPort_, uuid_, packPing.uuid_);
		XASSERT(device.ip_ == tmpIp_);
		XASSERT(device.port_ == tmpPort_);
		XASSERT(device.service_ == this);
		XASSERT(device.uuid_ == packPing.uuid_);
		XASSERT(std::memcmp(&device.udpAddress_, &address, sizeof(address)) == 0);
	}
	sendPong(address);
	device.sendStart();
}

void Service::onPong(const XUdpAddress& address, const UdpPackPong& packPong)
{
	UdpAddressToIpPort(address, tmpIp_, tmpPort_);
	if (uuid_ == packPong.uuid_)
	{
		XINFO("Pong reject native device. [%s:%d] uuid_=%lld, packPong.uuid_=%lld", tmpIp_.data(), tmpPort_, uuid_, packPong.uuid_);
		return;
	}
	auto& device = mapDevices_[address];
	if (device.uuid_ != packPong.uuid_)
	{
		XINFO("Pong find new device. [%s:%d] uuid_=%lld, packPong.uuid_=%lld", tmpIp_.data(), tmpPort_, uuid_, packPong.uuid_);
		if (device.uuid_ > 0)
		{
			device.onClose();
		}
		device.service_ = this;
		device.nativeUuid_ = uuid_;
		device.udpAddress_ = address;
		device.heartBeat_ = heartBeat_ / 1000 + 5;
		device.onOpen(EConnectDetectPong, packPong.uuid_, tmpIp_, tmpPort_);
		saveDevices();
	}
	else
	{
		XINFO("Pong already find device. [%s:%d] uuid_=%lld, packPong.uuid_=%lld", tmpIp_.data(), tmpPort_, uuid_, packPong.uuid_);
		XASSERT(device.ip_ == tmpIp_);
		XASSERT(device.port_ == tmpPort_);
		XASSERT(device.service_ == this);
		XASSERT(device.uuid_ == packPong.uuid_);
		XASSERT(std::memcmp(&device.udpAddress_, &address, sizeof(address)) == 0);
	}
	device.sendStart();
}

void Service::onHeart(const XUdpAddress& address, const UdpPackHeart& packHeart)
{
	auto iter = mapDevices_.find(address);
	if (iter == mapDevices_.end())
	{
		UdpAddressToIpPort(address, tmpIp_, tmpPort_);
		XINFO("Can't find device. [%s:%d] uuid_:%lld", tmpIp_.data(), tmpPort_, packHeart.uuid_);
		sendPing(address, false);
		return;
	}
	auto& device = iter->second;
	if (device.uuid_ != packHeart.uuid_)
	{
		UdpAddressToIpPort(address, tmpIp_, tmpPort_);
		XINFO("Device is changed. [%s:%d] device.uuid_:%lld, packHeart.uuid_:%lld", tmpIp_.data(), tmpPort_, device.uuid_, packHeart.uuid_);
		sendPing(address, false);
		return;
	}
	device.onHeartBeat(packHeart);
}

void Service::onTest(const XUdpAddress& address, const UdpPackTest& packTest)
{
	auto iter = mapDevices_.find(address);
	if (iter == mapDevices_.end())
	{
		UdpAddressToIpPort(address, tmpIp_, tmpPort_);
		XINFO("Can't find device. [%s:%d] uuid_:%lld", tmpIp_.data(), tmpPort_, packTest.uuid_);
		sendPing(address, false);
		return;
	}
	auto& device = iter->second;
	if (device.uuid_ != packTest.uuid_)
	{
		UdpAddressToIpPort(address, tmpIp_, tmpPort_);
		XINFO("Device is changed. [%s:%d] device.uuid_:%lld, packTest.uuid_:%lld", tmpIp_.data(), tmpPort_, device.uuid_, packTest.uuid_);
		sendPing(address, false);
		return;
	}
	device.onTest(packTest);
}

void Service::onData(const XUdpAddress& address, const char* data, uint16_t size)
{
	auto iter = mapDevices_.find(address);
	if (iter == mapDevices_.end())
	{
		UdpAddressToIpPort(address, tmpIp_, tmpPort_);
		XINFO("unknow device. [%s:%d] size:%d", tmpIp_.data(), tmpPort_, size);
		return;
	}
	auto& device = iter->second;
	if (device.service_ != this)
	{
		XASSERT(false);
		return;
	}
	XASSERT(std::memcmp(&device.udpAddress_, &address, sizeof(address)) == 0);
	device.onSocketData(data, size);
}

void Service::onSocketUdp(int fd, const char* data, uint16_t size, const XUdpAddress& address)
{
	if (size < sizeof(UdpPackHead))
	{
		UdpAddressToIpPort(address, tmpIp_, tmpPort_);
		XWARN("%s:%d size too small. size=%d data=%s\n", tmpIp_.data(), tmpPort_, (int)size, data);
		return;
	}
	const auto& packHead = (UdpPackHead*)data;
	switch (packHead->type_)
	{
	case EUdpPackPing:
	{
		if (size < sizeof(UdpPackPing))
		{
			UdpAddressToIpPort(address, tmpIp_, tmpPort_);
			XWARN("UdpPackPing[%s:%d] size too small. size=%d data=%s\n", tmpIp_.data(), tmpPort_, (int)size, data);
			return;
		}
		onPing(address, *(UdpPackPing*)data);
		return;
	}
	case EUdpPackPong:
	{
		if (size < sizeof(UdpPackPong))
		{
			UdpAddressToIpPort(address, tmpIp_, tmpPort_);
			XWARN("UdpPackPong[%s:%d] size too small. size=%d data=%s\n", tmpIp_.data(), tmpPort_, (int)size, data);
			return;
		}
		onPong(address, *(UdpPackPong*)data);
		return;
	}
	case EUdpPackHeart:
	{
		if (size < sizeof(UdpPackHeart))
		{
			UdpAddressToIpPort(address, tmpIp_, tmpPort_);
			XWARN("UdpPackHeart[%s:%d] size too small. size=%d data=%s\n", tmpIp_.data(), tmpPort_, (int)size, data);
			return;
		}
		onHeart(address, *(UdpPackHeart*)data);
		return;
	}
	//case EUdpPackTest:
	//{
	//	if (size < sizeof(UdpPackTest))
	//	{
	//		UdpAddressToIpPort(address, tmpIp_, tmpPort_);
	//		XWARN("EUdpPackTest[%s:%d] size too small. size=%d data=%s\n", tmpIp_.data(), tmpPort_, (int)size, data);
	//		return;
	//	}
	//	onTest(address, *(UdpPackTest*)data);
	//	return;
	//}
	default:
		break;
	}
	onData(address, data, size);
}

void Service::onSocketWarning(int fd, const char* info)
{
	XDEBUG("fd=%d, focusFd_=%d, info=%s", fd, focusFd_, info);
}

void Service::onSocketClose(int fd, const char* info)
{
	XDEBUG("fd=%d, focusFd_=%d, info=%s", fd, focusFd_, info);
}

Device* Service::getDevice(const XUdpAddress& address)
{
	auto iter = mapDevices_.find(address);
	if (iter == mapDevices_.end())
	{
		XASSERT(false);
		return 0;
	}
	auto& device = iter->second;
	XASSERT(std::memcmp(&device.udpAddress_, &address, sizeof(address)) == 0);
	return &device;
}

}

