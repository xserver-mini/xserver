#include "device.h"
#include "service.h"
#include "core/utils/common.h"
#include "service/udpport/event.h"
#include "utils/events.h"

namespace UdpDevice
{

Device::Device()
	:port_(0),
	isConnect_(false),
	service_(0),
	uuid_(0),
	status_(EDeviceStatusInit)
{
	sendSizeTotal_ = 0;
	sendSizePer_ = 0;
	receiveSizeTotal_ = 0;
	receiveSizePer_ = 0;
}

Device::~Device()
{
	clear();
}

void Device::clear()
{
	for (size_t i = 0; i < vectCache_.size(); i++)
	{
		delete vectCache_[i];
	}
	vectCache_.clear();
}

void Device::setAddress(const XUdpAddress& address)
{
	XServiceSocket::UdpAddressToIpPort(address, ip_, port_);
	udpAddress_ = address;
}

void Device::connect()
{
	XASSERT(false);
	if (!service_)
	{
		XASSERT(false);
		return;
	}
	auto sevent = new EventUdpState;
	sevent->estate_ = EUdpStateConnect;
	sevent->address_ = udpAddress_;
	sevent->uuid_ = 0;
	service_->sendEvent(sevent, ERobotIDUdpPort);
}

void Device::openCtrl()
{
	MsgPrepareAudio msg;
	//msg.waveFormat_ = service_->ctrl_.waveFormat_;
	sendData(EMsgPrepareAudio, (const char*)&msg, sizeof(msg));
}

void Device::sendData(uint16_t msgId, const char* data, size_t size)
{
	auto sevent = new EventUdpData;
	sevent->estate_ = EUdpStateSend;
	sevent->address_ = udpAddress_;
	sevent->uid_ = uuid_;
	sevent->msgId_ = msgId;
	if (data && size > 0)
	{
		sevent->getBuffer()->append(data, size);
	}
	service_->sendEvent(sevent, ERobotIDUdpPort);
	sendSizeTotal_ += size + sizeof(uint16_t);
	sendSizePer_ += size + sizeof(uint16_t);
}

void Device::onData(const EventUdpData& event)
{
	uint16_t msgId = event.msgId_;
	const char* data = event.getBuffer()->data();
	size_t size = event.getBuffer()->size();
	//XDEBUG("msgId=%d, size=%d, data=%s, [%s]", msgId, (int)size, data, OpenTime::MilliToString(OpenTime::MilliUnixtime()).data());
	if (EMsgPrepareAudio == msgId)
	{
		XDEBUG("[Player]Player is Ready, tell the recorder.");
		if (size < sizeof(MsgPrepareAudio))
		{
			XASSERT(false);
			return;
		}
		auto msg = (MsgPrepareAudio*)data;
		//waveFormat_ = msg->waveFormat_;

		MsgReadyAudio sendMsg;
		//sendMsg.waveFormat_ = waveFormat_;
		sendData(EMsgReadyAudio, (const char*)&sendMsg, sizeof(sendMsg));
	}
	else if (EMsgReadyAudio == msgId)
	{
		status_ = EDeviceStatusPlaying;
		if (size < sizeof(MsgReadyAudio))
		{
			XASSERT(false);
			return;
		}
		auto msg = (MsgReadyAudio*)data;
		//XASSERT(std::memcmp(&msg->waveFormat_, &service_->ctrl_.waveFormat_, sizeof(msg->waveFormat_)) == 0);

		XDEBUG("[Recorder]Player is Ok, send audio");
		reqId_ = 0;
	}
	else if (EMsgPlayAudio == msgId)
	{
		XDEBUG("[Player]Receive Recorder start msg");
		if (size < sizeof(MsgPlayAudio))
		{
			XASSERT(false);
			return;
		}
		auto msg = (MsgPlayAudio*)data;
		//XASSERT(std::memcmp(&msg->waveFormat_, &waveFormat_, sizeof(waveFormat_)) == 0);
	}
	else if (EMsgSendAudio == msgId)
	{
		if (size < sizeof(MsgSendAudio))
		{
			XASSERT(false);
			return;
		}
		auto msg = (MsgSendAudio*)data;
		if (msg->isLast_)
		{
			if (vectCache_.empty())
			{
				data += sizeof(MsgSendAudio);
				size -= sizeof(MsgSendAudio);
				XASSERT(size % msg->sampleSize_ == 0);
			}
			else
			{
				XASSERT(cacheReqId_ == msg->reqId_);
				std::string buffer;
				const char* tmpData;
				size_t tmpSize;
				for (size_t i = 0; i < vectCache_.size(); i++)
				{
					auto& cache = vectCache_[i];
					tmpData = cache->data() + sizeof(MsgSendAudio);
					tmpSize = cache->size() - sizeof(MsgSendAudio);
					buffer.append(tmpData, tmpSize);
					delete cache;
				}
				vectCache_.clear();
				data += sizeof(MsgSendAudio);
				size -= sizeof(MsgSendAudio);
				buffer.append(data, size);
				XASSERT(size % msg->sampleSize_ == 0);
			}
		}
		else
		{
			if (vectCache_.empty())
			{
				cacheReqId_ = msg->reqId_;
			}
			else
			{
				XASSERT(cacheReqId_ == msg->reqId_);
			}
			vectCache_.push_back(((EventUdpData*)&event)->swap());
		}
	}
	else if (EMsgStopAudio == msgId)
	{
		XDEBUG("[Player]Receive Recorder stop msg");
	}
	else if (EMsgTest == msgId)
	{
		if (size < sizeof(MsgTest))
		{
			XASSERT(false);
			return;
		}
		auto msg = (MsgTest*)data;
		onTest(*msg);
	}
	receiveSizeTotal_ += size + sizeof(uint16_t);
	receiveSizePer_ += size + sizeof(uint16_t);
}

void Device::onOpen(int64_t uuid)
{
	//XDEBUG("onOpen===>>");
	isConnect_ = true;
	uuid_ = uuid;

//#ifdef __ANDROID__
//#else
//	sendTest(10000);
//#endif // __ANDROID__
}

void Device::onClose()
{
	//XDEBUG("onClose===>>");
	uuid_ = 0;
	isConnect_ = false;
	status_ = EDeviceStatusDisconnet;

	clear();
}

void Device::onError(const char* data)
{
	XDEBUG("onError===>>%s", data);
}

void Device::sendTest(int count)
{
	testCount_ = count;

	MsgTest msg;
	msg.count_ = count;
	msg.isReq_ = 1;
	testStartTime_ = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	msg.time_ = testStartTime_;
	sendData(EMsgTest, (const char*)&msg, sizeof(msg));
	XDEBUG("sendTest=======================>> count=%[ d ]", count);

}

void Device::onTest(const MsgTest& msgTest)
{
	if (msgTest.isReq_ == 1)
	{
		MsgTest msg;
		msg.isReq_ = 0;
		msg.count_ = msgTest.count_;
		msg.time_  = msgTest.time_;
		sendData(EMsgTest, (const char*)&msg, sizeof(msg));
		//XDEBUG("onTest=======================>> msgTest.count_=[ %d ]", msgTest.count_);
	}
	else
	{
		auto currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		if (msgTest.count_ > 1)
		{
			MsgTest msg;
			msg.count_ = msgTest.count_ - 1;
			msg.isReq_ = 1;
			msg.time_ = currentTime;
			//XDEBUG("sendTest=======================>> msg.count_=[ %d ], [%ld]:%lld ms", msg.count_, msgTest.count_, currentTime - msgTest.time_);
			sendData(EMsgTest, (const char*)&msg, sizeof(msg));
			//if (msgTest.count_ % 10 == 0)
		}
		else
		{
			XDEBUG("[%d]==========totalCount:[ %d ], totalCost:%lld, cost:%lld ms", msgTest.count_, testCount_, (currentTime - testStartTime_), (currentTime - testStartTime_) / testCount_);
		}
	}
}

}

