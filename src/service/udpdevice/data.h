#pragma once

#include <cstdint>

enum EDeviceStatus
{
	EDeviceStatusNone,
	EDeviceStatusInit,
	EDeviceStatusPlaying,
	EDeviceStatusDisconnet,
};

enum EMsgId
{
	EMsgNone,
	EMsgPrepareAudio,
	EMsgReadyAudio,
	EMsgPlayAudio,
	EMsgSendAudio,
	EMsgStopAudio,
	EMsgRejectAudio,
	EMsgTest
};

// A EMsgPrepareAudio => B
// A <<= EMsgReadyAudio B
// A  EMsgPlayAudio => B
// A  EMsgSendAudio => B Repeat
// A  EMsgStopAudio => B
// A <<= EMsgReadyAudio B


// msg
#ifdef __cplusplus
extern "C" {
#endif
#pragma pack(1)

struct MsgPrepareAudio
{
	int16_t code_;
	//XWaveFormat waveFormat_;
};

struct MsgReadyAudio
{
	int16_t code_;
	//XWaveFormat waveFormat_;
};

struct MsgPlayAudio
{
	int16_t code_;
	//XWaveFormat waveFormat_;
};

struct MsgSendAudio
{
	uint16_t reqId_;
	uint8_t isLast_;
	uint8_t sampleSize_;
};

struct MsgStopAudio
{
	int16_t code_;
};

struct MsgRejectAudio
{
	int16_t code_;
};

struct MsgTest
{
	int32_t count_;
	int32_t isReq_;
	int64_t time_;
};



#pragma pack()
#ifdef __cplusplus
}
#endif