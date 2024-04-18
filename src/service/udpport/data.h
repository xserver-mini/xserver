#pragma once

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif
#pragma pack(1)

struct UdpPackHead
{
	uint8_t type_;
	int32_t sidx_;
	int32_t ridx_;
	uint16_t len_;
};

struct UdpPackPing
{
	UdpPackHead header_;
	int64_t uuid_;
	uint8_t isTest_;
	uint8_t reserved_[32];
};

struct UdpPackPong
{
	UdpPackHead header_;
	int64_t uuid_;
	uint8_t reserved_[32];
};

struct UdpPackStart
{
	UdpPackHead header_;
	int64_t uuid_;
};

struct UdpPackStop
{
	UdpPackHead header_;
	int64_t uuid_;
};

struct UdpPackHeart
{
	UdpPackHead header_;
	int64_t uuid_;
	uint8_t reserved_[64];
};

struct UdpPackTest
{
	UdpPackHead header_;
	int64_t uuid_;
	int32_t count_;
	int32_t isReq_;
	int64_t time_;
};

struct UdpPackRepeat
{
	UdpPackHead header_;
	uint8_t isResp_;
	uint16_t len_;
	int32_t list_[256];
};

struct UdpPackData
{
	UdpPackHead header_;
	uint16_t msgId_;
};

#pragma pack()
#ifdef __cplusplus
}
#endif

enum EUdpPackId
{
	EUdpPackNone   = 0,
	EUdpPackPing   = 1,
	EUdpPackPong   = 2,
	EUdpPackStart  = 3,
	EUdpPackHeart  = 4,
	EUdpPackRepeat = 5,
	EUdpPackTest   = 6,
	EUdpPackData   = 7,
};

struct UdpPackage
{
	int32_t idx_ = 0;
	bool isOK_ = false;
	std::string buffer_;
};