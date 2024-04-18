#include "device.h"
#include "service.h"
#include "event.h"
#include "core/utils/common.h"
#include "core/platform/net.h"

namespace UdpPort
{

Device::Device()
	:port_(0),
	beatCount_(0),
	service_(0),
	isConnect_(false)
{
}

Device::~Device()
{
	clearList();
}

void Device::clearList()
{
	XINFO("===========>>clearList");
	for (auto iter = sendList_.begin(); iter != sendList_.end(); iter++)
	{
		delete* iter;
	}
	sendList_.clear();

	for (auto iter = recevieList_.begin(); iter != recevieList_.end(); iter++)
	{
		delete* iter;
	}
	recevieList_.clear();
}

// exclude idx
void Device::removeSendList(int32_t otherReceivedx)
{
	for (auto iter = sendList_.begin(); iter != sendList_.end();)
	{
		auto& pack = *iter;
		if (otherReceivedx > 0 && pack->idx_ < 0)
		{
			break;
		}
		if (pack->idx_ <= otherReceivedx)
		{
#ifdef UDP_MSG_DEBUG
			XINFO("removeSendList sendNextIdx_=%d, otherReceivedx=%d, remove send idx_=%d", sendNextIdx_, otherReceivedx, pack->idx_);
#endif // UDP_MSG_DEBUG

			delete pack;
			iter = sendList_.erase(iter);
			continue;
		}
		if (pack->idx_ > 0 && otherReceivedx < 0)
		{
#ifdef UDP_MSG_DEBUG
			XINFO("removeSendList sendNextIdx_=%d, otherReceivedx=%d, remove send idx_=%d", sendNextIdx_, otherReceivedx, pack->idx_);
#endif // UDP_MSG_DEBUG

			delete pack;
			iter = sendList_.erase(iter);
			continue;
		}
		break;
	}
}
void Device::sendPackage(UdpPackage* package)
{
	if (!package)
	{
		XASSERT(false);
		return;
	}
	auto& buffer = package->buffer_;
	if (buffer.size() >= 1472)
	{
		XASSERT(false);
		return;
	}
	if (buffer.size() < sizeof(UdpPackData))
	{
		XASSERT(false);
		return;
	}
	UdpPackHead* packHead = (UdpPackHead*)buffer.data();
	packHead->type_ = EUdpPackData;
	packHead->sidx_ = sendNextIdx_;
	packHead->ridx_ = receiveIdx_;
	packHead->len_ = (uint16_t)(buffer.size() - sizeof(UdpPackHead));

	package->idx_ = sendNextIdx_;
	package->isOK_ = false;
	sendList_.push_back(package);
	sendNextIdx_++;
	sendSocketData(buffer.data(), (uint16_t)buffer.size());
}

//idx size data 2 2 xx
void Device::sendData(uint16_t msgId, const char* data, uint16_t size)
{
	if (size + sizeof(UdpPackData) >= 1472)
	{
		XASSERT(false);
		return;
	}
	tmpPackData_.header_.type_ = EUdpPackData;
	tmpPackData_.header_.sidx_ = sendNextIdx_;
	tmpPackData_.header_.ridx_ = receiveIdx_;
	tmpPackData_.header_.len_  = size + sizeof(tmpPackData_.msgId_);
	tmpPackData_.msgId_ = msgId;

	auto package = new UdpPackage;
	package->idx_ = sendNextIdx_;
	package->isOK_ = false;
	auto& buffer = package->buffer_;
	buffer.reserve(size + sizeof(UdpPackData));
	buffer.append((const char*)&tmpPackData_, sizeof(UdpPackData));
	if (size > 0 && data)
	{
		buffer.append((const char*)data, size);
	}
	sendList_.push_back(package);
	sendNextIdx_++;
	sendSocketData(buffer.data(), (uint16_t)buffer.size());

	//const auto& packHead = tmpPackData_.header_;
	//XDEBUG("[%s:%d]%s sidx=%d, ridx=%d, len=%d, sendNextIdx_=%d, receiveIdx_=%d, size=%d", ip_.data(), port_,
	//	GetUdpPackName(packHead.type_), packHead.sidx_, packHead.ridx_, packHead.len_, sendNextIdx_, receiveIdx_, size);
}

void Device::onData(const char* data, uint16_t size)
{
	//XWARN("==============>> sendNextIdx_=%d, receiveIdx_=%d", sendNextIdx_, receiveIdx_);
	if (size + sizeof(UdpPackHead) < sizeof(UdpPackData))
	{
		XASSERT(false);
		return;
	}
	auto sevent = new EventUdpData;
	sevent->estate_ = EUdpStateReceive;
	sevent->msgId_ = ((UdpPackData*)data)->msgId_;
	auto offetSize = sizeof(UdpPackData);
	sevent->getBuffer()->append(data + offetSize, size - offetSize);
	sendEvent(sevent);
}

bool Device::sendEvent(EventUdpState* event)
{
	if (service_)
	{
		event->uuid_ = uuid_;
		event->address_ = udpAddress_;
		service_->sendEvent(event, ERobotIDUdpDevice);
		return true;
	}
	delete event;
	return false;	
}

void Device::onOpen(EConnectDetect eDetect, int64_t uuid, const std::string& ip, int port)
{
	XINFO("[%s:%d]=======>> uuid=%lld, nativeUuid_=%lld", ip.data(), port, uuid, nativeUuid_);
	XASSERT(!uuid_);
	connectDetect_ |= eDetect;
	uuid_ = uuid;
	ip_ = ip;
	port_ = port;
	sendNextIdx_ = 1;
	receiveIdx_ = 0;
	lastTime_ = OpenTime::Unixtime();
	isConnect_ = true;

	auto sevent = new EventUdpState;
	sevent->estate_ = EUdpStateOpen;
	sendEvent(sevent);

	if (OpenTime::Unixtime() > lastTime_ + 12)
	{
		clearList();
	}

	//sendNextIdx_ = 0x7fffffff - 10;
	//receiveIdx_ = sendNextIdx_ - 1;

	//auto isTest = XConfig::GetInstance().getValueInt("default", "test", 0);
	//if (!isTest)
	//{
	//	return;
	//}
//
//#ifdef __ANDROID__
//#else
//	sendTest(10000);
//#endif // __ANDROID__

}

void Device::onClose()
{
	XINFO("=======>>");
	if (uuid_ > 0)
	{
		auto sevent = new EventUdpState;
		sevent->estate_ = EUdpStateClose;
		sendEvent(sevent);
	}
	uuid_ = 0;
	isConnect_ = false;
	connectDetect_ = 0;
	if (OpenTime::Unixtime() > lastTime_ + 12)
	{
		clearList();
	}
}

//1472
void Device::sendSocketData(const char* data, uint16_t size)
{
	if (size < sizeof(UdpPackHead))
	{
		XASSERT(false);
		return;
	}
	if (size >= 1472)
	{
		XASSERT(false);
		return;
	}
#ifdef UDP_MSG_DEBUG
	const auto& packHead = *(UdpPackHead*)data;
	XDEBUG("[%s:%d]%s sidx=%d, ridx=%d, len=%d, sendNextIdx_=%d, receiveIdx_=%d, size=%d", ip_.data(), port_,
		GetUdpPackName(packHead.type_), packHead.sidx_, packHead.ridx_, packHead.len_, sendNextIdx_, receiveIdx_, size);
#endif // UDP_MSG_DEBUG
	service_->sendUdpSocket(service_->focusFd_, data, size, udpAddress_);
}

void Device::onSocketData(const char* data, uint16_t size)
{
	if (size < sizeof(UdpPackData))
	{
		XWARN("[%s:%d]size=%lld, sizeof(UdpPackHead)=%lld", ip_.data(), port_, size, sizeof(UdpPackHead));
		auto sevent = new EventUdpError;
		sevent->estate_ = EUdpStateError;
		sevent->buffer_ = "size < " + std::to_string(sizeof(UdpPackHead)) + ". size=" + std::to_string(size);
		sendEvent(sevent);
		XWARN("=======================================>>caution Insufficient data");
		return;
	}
	const auto& packHead = *(UdpPackHead*)data;
#ifdef UDP_MSG_DEBUG
	XDEBUG("[%s:%d]%s sidx=%d, ridx=%d, len=%d, sendNextIdx_=%d, receiveIdx_=%d, size=%d", ip_.data(), port_,
		GetUdpPackName(packHead.type_), packHead.sidx_, packHead.ridx_, packHead.len_, sendNextIdx_, receiveIdx_, size);
#endif // UDP_MSG_DEBUG
	removeSendList(packHead.ridx_);
	//lastTime_ = OpenTime::Unixtime();
	if (packHead.type_ == EUdpPackStart)
	{
		if (size < sizeof(UdpPackStart))
		{
			XWARN("[%s:%d]size=%lld, sizeof(UdpPackStart)=%lld", ip_.data(), port_, size, sizeof(UdpPackStart));
			auto sevent = new EventUdpError;
			sevent->estate_ = EUdpStateError;
			sevent->buffer_ = "size < " + std::to_string(sizeof(UdpPackStart)) + ". size=" + std::to_string(size);
			sendEvent(sevent);
			XWARN("=======================================>>caution Start Insufficient data");
			return;
		}
		onStart(*(UdpPackStart*)data);
		return;
	}

	if (packHead.type_ == EUdpPackRepeat)
	{
		if (size < sizeof(UdpPackRepeat))
		{
			XWARN("[%s:%d]size=%lld, sizeof(UdpPackRepeat)=%lld", ip_.data(), port_, size, sizeof(UdpPackRepeat));
			auto sevent = new EventUdpError;
			sevent->estate_ = EUdpStateError;
			sevent->buffer_ = "size < " + std::to_string(sizeof(UdpPackRepeat)) + ". size=" + std::to_string(size);
			sendEvent(sevent);
			XWARN("=======================================>>caution Repeat Insufficient data");
			return;
		}
		onRepeat(*(UdpPackRepeat*)data);
		return;
	}

	if (packHead.type_ != EUdpPackData)
	{
		XWARN("[%s:%d]type_!=EUdpPackData. EUdpPackData=%d, type_=%d", ip_.data(), port_, EUdpPackData, packHead.type_);
		auto sevent = new EventUdpError;
		sevent->estate_ = EUdpStateError;
		sevent->buffer_ = "type_ != EUdpPackData. type_=" + std::to_string(packHead.type_);
		sendEvent(sevent);
		XWARN("======================================>>caution No packdata");
		return;
	}
	const auto& len = packHead.len_;
	if (sizeof(UdpPackHead) + len != size)
	{
		XWARN("[%s:%d]sizeof(UdpPackHead)+len!=size. size=%lld, len=%d, sizeof(UdpPackHead)=%lld", ip_.data(), port_, size, len, sizeof(UdpPackHead));
		auto sevent = new EventUdpError;
		sevent->estate_ = EUdpStateError;
		sevent->buffer_ = "len + " + std::to_string(sizeof(UdpPackHead)) + " != size. size=" + std::to_string(size) + ", len=" + std::to_string(len);
		sendEvent(sevent);
		XWARN("========================================>>caution Data size is error");
		return;
	}

	//int32_t otherSendIdx = packHead.sidx_;
	if (packHead.sidx_ <= receiveIdx_)
	{
		if (!(packHead.sidx_ < 0 && receiveIdx_ > 0))
		{
			//XWARN("drop data1. receiveIdx_=%d,  packHead.sidx_=%d", receiveIdx_, packHead.sidx_);
			//XWARN("=======================================>>caution sidx less than ridx drop data");
			return;
		}
	}
	else
	{
		if (packHead.sidx_ > 0 && receiveIdx_ < 0)
		{
			//XWARN("drop data2. receiveIdx_=%d,  packHead.sidx_=%d", receiveIdx_, packHead.sidx_);
			XWARN("=======================================>>caution sidx less than ridx drop data");
			return;
		}
	}

	if (size < sizeof(UdpPackData))
	{
		XWARN("[%s:%d]tlen!=size. size=%lld, len=%d", ip_.data(), port_, size, len);
		auto sevent = new EventUdpError;
		sevent->estate_ = EUdpStateError;
		sevent->buffer_ = "len != size. size=" + std::to_string(size) + ", len=" + std::to_string(len);
		sendEvent(sevent);
		XWARN("======================================>>caution size is less than packdata");
		return;
	}
	if (packHead.sidx_ == receiveIdx_ + 1)
	{
		//ondata
		++receiveIdx_;
		XASSERT(packHead.sidx_ == receiveIdx_);
		onData(data, size);
		if (recevieList_.empty())
		{
			return;
		}
		auto iter = recevieList_.begin();
		while (iter != recevieList_.end())
		{
			auto& pack = *iter;
			if (pack->idx_ <= receiveIdx_)
			{
				delete pack;
				iter = recevieList_.erase(iter);
				XWARN("=====================================>>caution drop already cache data");
				continue;
			}
			else
			{
				if (receiveIdx_ < 0 && pack->idx_ > 0)
				{
					delete pack;
					iter = recevieList_.erase(iter);
					XWARN("=====================================>>caution drop already cache data");
					continue;
				}
			}
			break;
		}
		for (iter = recevieList_.begin(); iter != recevieList_.end();)
		{
			auto& pack = *iter;
			if (pack->idx_ == receiveIdx_ + 1)
			{
				++receiveIdx_;
				XASSERT(receiveIdx_ == pack->idx_);
				onData(pack->buffer_.data(), (uint16_t)pack->buffer_.size());
				delete pack;
				iter = recevieList_.erase(iter);
				XWARN("=====================================>>caution for get exist data");
				continue;
			}
			break;
		}
		if (!recevieList_.empty())
		{
			sendRepeat();
		}
		return;
	}
	
	bool isOk = false;
	auto& sidx = packHead.sidx_;
	for (auto iter = recevieList_.begin(); iter != recevieList_.end(); iter++)
	{
		auto& pack = *iter;
		if (pack->idx_ == sidx)
		{
			if (pack->buffer_.size() == size)
			{
				XASSERT(std::memcmp(pack->buffer_.data(), data, size) == 0);
			}
			else
			{
				pack->buffer_.clear();
				pack->buffer_.append(data, size);
				XWARN("=====================================>>caution size error.");
				XASSERT(false);
			}
			isOk = true;
			break;
		}
		if (pack->idx_ > 0 && sidx < 0)
		{
			continue;
		}
		//1, 2, _, 4, 5
		if (pack->idx_ > sidx)
		{
			auto package = new UdpPackage;
			package->idx_ = sidx;
			package->buffer_.append(data, size);
			recevieList_.insert(iter, package);
			isOk = true;
			break;
		}
	}
	if (!isOk)
	{
		auto package = new UdpPackage;
		package->idx_ = sidx;
		package->buffer_.append(data, size);
		recevieList_.push_back(package);
	}
	sendRepeat();
}

void Device::sendStart()
{
	lastTime_ = OpenTime::Unixtime();
	UdpPackStart packStart;
	auto& header = packStart.header_;
	header.type_ = EUdpPackStart;
	header.sidx_ = sendNextIdx_;
	header.ridx_ = receiveIdx_;
	packStart.uuid_ = nativeUuid_;
	sendSocketData((const char*)&packStart, sizeof(packStart));
}

void Device::onStart(const UdpPackStart& packStart)
{
	receiveIdx_ = packStart.header_.sidx_ - 1;
	if (sendNextIdx_ > packStart.header_.ridx_ + 1)
	{
		sendRepeat();
	}
}

void Device::sendHeartBeat()
{
	XINFO("[%s:%d] sendNextIdx_=%d, receiveIdx_=%d, send[%lld], receive[%lld]", 
		ip_.data(), port_, sendNextIdx_, receiveIdx_, sendList_.size(), recevieList_.size());
	XASSERT(nativeUuid_ != uuid_);

	UdpPackHeart packHeart;
	auto& header = packHeart.header_;
	header.type_ = EUdpPackHeart;
	header.sidx_ = sendNextIdx_;
	header.ridx_ = receiveIdx_;
	packHeart.uuid_ = nativeUuid_;
	sendSocketData((const char*)&packHeart, sizeof(packHeart));

	if (OpenTime::Unixtime() > lastTime_ + heartBeat_)
	{
		//XDEBUG("Can't find device. %s:%d", ip_.data(), port_);
		beatCount_++;
		if (beatCount_ < 6)
		{
			XINFO("wait ==>> %s:%d beatCount_=%d", ip_.data(), port_, beatCount_);
			return;
		}
		beatCount_ = 0;
		if (isConnect_)
		{
			isConnect_ = false;
			//notice close
			//onClose();
		}
	}
}

void Device::onHeartBeat(const UdpPackHeart& packHeart)
{
	auto& header = packHeart.header_;
	if (header.type_ != EUdpPackHeart)
	{
		XASSERT(false);
		return;
	}
	XINFO("[%s:%d] header.sidx_=%d, header.ridx_=%d,  sendNextIdx_=%d, receiveIdx_=%d, send[%lld], receive[%lld]", 
						ip_.data(), port_, header.sidx_, header.ridx_, sendNextIdx_, receiveIdx_, sendList_.size(), recevieList_.size());
	lastTime_ = OpenTime::Unixtime();
	removeSendList(header.ridx_);

	if (header.ridx_ != sendNextIdx_ - 1)
	{
		if (header.ridx_ > 0 && sendNextIdx_ < 0)
		{
			sendRepeat();
		}
		else if(sendNextIdx_ > header.ridx_)
		{
			for (auto iter = sendList_.begin(); iter != sendList_.end(); iter++)
			{
				auto& pack = *iter;
				auto& buffer = pack->buffer_;
				sendSocketData(buffer.data(), (uint16_t)buffer.size());
			}
		}
	}
	
	if (header.sidx_ != receiveIdx_ + 1)
	{
		if (header.sidx_ < 0 && receiveIdx_ > 0)
		{
			sendHeartBeat();
		}
		else if (header.sidx_ > receiveIdx_)
		{
			sendHeartBeat();
		}
	}
	//XINFO("[%s:%d] header.sidx_=%d, header.ridx_=%d,  sendNextIdx_=%d, receiveIdx_=%d",
	//	ip_.data(), port_, header.sidx_, header.ridx_, sendNextIdx_, receiveIdx_);
}

void Device::sendRepeat()
{
	if (recevieList_.empty())
	{
		//XASSERT(false);
		return;
	}
	UdpPackRepeat packRepeat = {0};
	packRepeat.header_.type_ = EUdpPackRepeat;
	packRepeat.header_.sidx_ = sendNextIdx_;
	packRepeat.header_.ridx_ = receiveIdx_;
	packRepeat.header_.len_ = 0;
	packRepeat.isResp_ = 0;
	packRepeat.len_ = 0;

	int32_t receiveIdx = receiveIdx_ + 1;
	uint16_t maxCount = sizeof(packRepeat.list_) / sizeof(packRepeat.list_[0]);
	for (auto iter = recevieList_.begin(); iter != recevieList_.end(); iter++)
	{
		auto& pack = *iter;
		while (true)
		{
			if (receiveIdx < pack->idx_)
			{
				packRepeat.list_[packRepeat.len_] = receiveIdx;
				//XINFO("sendRepeat1 lock ridx=%d", receiveIdx);
				packRepeat.len_++;
				++receiveIdx;
				if (packRepeat.len_ >= maxCount)
				{
					break;
				}
				continue;
			}
			else
			{
				if (receiveIdx > 0 && pack->idx_ < 0)
				{
					packRepeat.list_[packRepeat.len_] = receiveIdx;
					//XINFO("sendRepeat2 lock ridx=%d", receiveIdx);
					packRepeat.len_++;
					++receiveIdx;
					if (packRepeat.len_ >= maxCount)
					{
						break;
					}
					continue;
				}
			}
			break;
		}
		if (packRepeat.len_ >= maxCount)
		{
			break;
		}
	}
	if (packRepeat.len_ == 0)
	{
		XASSERT(false);
		return;
	}
	//XINFO("sendRepeat sendNextIdx_=%d, receiveIdx_=%d", sendNextIdx_, receiveIdx_);
	sendSocketData((const char*)&packRepeat, sizeof(packRepeat));
}

void Device::onRepeat(const UdpPackRepeat& packRepeat)
{
	//XWARN("=====sidx_=%d, ridx_=%d", packRepeat.header_.sidx_, packRepeat.header_.ridx_);
	if (packRepeat.isResp_)
	{
		receiveIdx_ = packRepeat.header_.sidx_ - 1;
		return;
	}
	if (packRepeat.len_ == 0)
	{
		return;
	}
	if (!sendList_.empty())
	{
		int32_t sendIdx = (*sendList_.begin())->idx_;
		int32_t otherRIdx = packRepeat.header_.ridx_ + 1;
		if (sendIdx > otherRIdx)
		{
			UdpPackRepeat pack;
			pack.header_.sidx_ = sendIdx;
			pack.isResp_ = 1;
			sendSocketData((const char*)&pack, sizeof(pack));
		}
		else if (sendIdx < 0 && otherRIdx > 0)
		{
			UdpPackRepeat pack;
			pack.header_.sidx_ = sendIdx;
			pack.isResp_ = 1;
			sendSocketData((const char*)&pack, sizeof(pack));
		}
	}

	tmpSetSidx_.clear();
	for (uint16_t i = 0; i < packRepeat.len_; i++)
	{
		tmpSetSidx_.insert(packRepeat.list_[i]);
	}
	for (auto iter = sendList_.begin(); iter != sendList_.end(); iter++)
	{
		auto& pack = *iter;
		auto iterFind = tmpSetSidx_.find(pack->idx_);
		if (iterFind != tmpSetSidx_.end())
		{
			//XINFO("onRepeat resend pack->idx_=%d", pack->idx_);
			auto& buffer = pack->buffer_;
			sendSocketData(buffer.data(), (uint16_t)buffer.size());
			tmpSetSidx_.erase(iterFind);
			if (tmpSetSidx_.empty())
			{
				break;
			}
		}
	}
}

void Device::sendTest(int count)
{
	testCount_ = count;
	UdpPackTest packTest;
	packTest.header_.type_ = EUdpPackTest;
	packTest.uuid_ = service_->uuid_;
	packTest.count_ = count;
	packTest.isReq_ = 1;
	testStartTime_ = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	packTest.time_ = testStartTime_;
	sendSocketData((const char*)&packTest, (uint16_t)sizeof(packTest));
}

void Device::onTest(const UdpPackTest& packTest)
{
	if (packTest.isReq_ == 1)
	{
		UdpPackTest pack;
		pack.header_.type_ = EUdpPackTest;
		pack.uuid_ = service_->uuid_;
		pack.count_ = packTest.count_;
		pack.isReq_ = 0;
		pack.time_ = packTest.time_;
		sendSocketData((const char*)&pack, (uint16_t)sizeof(pack));
	}
	else
	{
		auto currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		if (packTest.count_ > 1)
		{
			UdpPackTest pack;
			pack.header_.type_ = EUdpPackTest;
			pack.uuid_ = service_->uuid_;
			pack.count_ = packTest.count_ - 1;
			pack.isReq_ = 1;
			pack.time_ = currentTime;
			sendSocketData((const char*)&pack, (uint16_t)sizeof(pack));

			XDEBUG("[%d]cost:%ld ms", packTest.count_, currentTime - packTest.time_);
		}
		else
		{
			XDEBUG("[%d]totalCount:%lld, totalCost:%lld, cost:%lld ms", packTest.count_, testCount_, (currentTime - testStartTime_), (currentTime - testStartTime_) / testCount_);
		}
	}
}

const char* Device::GetUdpPackName(int packId)
{
	switch (packId)
	{
	case EUdpPackNone:
		return "PackNone";
	case EUdpPackPing:
		return "PackPing";
	case EUdpPackPong:
		return "PackPong";
	case EUdpPackStart:
		return "PackStart";
	case EUdpPackHeart:
		return "PackHeart";
	case EUdpPackRepeat:
		return "PackRepeat";
	case EUdpPackData:
		return "PackData";
	default:
		break;
	}
	return "PackUnknown";
}

}

