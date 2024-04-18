/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/

#include "openbuffer.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#ifndef DATA_BIG_ENDIAN
#ifdef _BIG_ENDIAN_
#if _BIG_ENDIAN_
#define DATA_BIG_ENDIAN 1
#endif
#endif
#ifndef DATA_BIG_ENDIAN
#if defined(__hppa__) || \
            defined(__m68k__) || defined(mc68000) || defined(_M_M68K) || \
            (defined(__MIPS__) && defined(__MIPSEB__)) || \
            defined(__ppc__) || defined(__POWERPC__) || defined(_M_PPC) || \
            defined(__sparc__) || defined(__powerpc__) || \
            defined(__mc68000__) || defined(__s390x__) || defined(__s390__)
#define DATA_BIG_ENDIAN 1
#endif
#endif
#ifndef DATA_BIG_ENDIAN
#define DATA_BIG_ENDIAN  0
#endif
#endif

namespace open
{
union TestUnion
{
	uint8_t val1;
	uint8_t val2[2];
};
static bool TestMask = false;
static void TestEndian()
{
	TestMask = true;
	TestUnion test = {0};
	test.val1 = 1;
#if DATA_BIG_ENDIAN
	assert(test.val2[1] == test.val1);
#else
	assert(test.val2[0] == test.val1);
#endif
}


OpenBuffer::OpenBuffer()
	:size_(0),
	offset_(0),
	cap_(0),
	buffer_(NULL),
	miniCap_(16)
{
	if (!TestMask) TestEndian();
}

OpenBuffer::OpenBuffer(size_t capacity)
	:size_(0),
	offset_(0),
	cap_(0),
	buffer_(NULL),
	miniCap_(capacity)
{
	if (!TestMask) TestEndian();
}

OpenBuffer::~OpenBuffer()
{
	clear();
	if (buffer_)
	{
		delete buffer_;
		buffer_ = NULL;
	}
}

const char* OpenBuffer::data()
{ 
	if (!buffer_)
	{
		assert(size_ == 0);
		size_ = 0;
		cap_ = miniCap_;
		buffer_ = new unsigned char[cap_ + 2];
		buffer_[0] = 0;
		return (const char*)buffer_;
	}
	if (offset_ + size_ <= cap_)
	{
		buffer_[offset_ + size_] = 0;
	}
	else
	{
		buffer_[cap_] = 0;
		assert(false);
	}
	return (const char*)(buffer_ + offset_); 
}

void OpenBuffer::setData(const char* data, size_t len)
{
	clearResize(len);
	offset_ = 0;
	size_ = 0;
	writeBack(data, len);
}

const char* OpenBuffer::clearResize(size_t size)
{
	if (cap_ >= size)
	{
		if (!buffer_)
		{
			buffer_ = new unsigned char[cap_ + 2];
		}
	}
	else 
	{
		if (buffer_)
		{
			delete buffer_;
		}
		cap_ = size;
		buffer_ = new unsigned char[cap_ + 2];
	}
	offset_ = 0;
	size_ = size;
	buffer_[cap_] = 0;
	return (const char*)buffer_;
}

void OpenBuffer::clear()
{
	size_    = 0;
	offset_  = 0;
	//cap_     = 0;
}

int64_t OpenBuffer::readFront(void* data, size_t len)
{
	if (size_ < len)
	{
		return -1;
	}
	if (!buffer_)
	{
		assert(false);
		return -1;
	}
	if (data)
	{
		memcpy(data, buffer_ + offset_, len);
	}
	offset_ += len;
	size_   -= len;
	return size_;
}

int64_t OpenBuffer::readBack(void* data, size_t len)
{
	if (size_ < len)
	{
		return -1;
	}
	if (!buffer_)
	{
		assert(false);
		return -1;
	}
	if (data)
	{
		memcpy(data, buffer_ + offset_ + size_ - len, len);
	}
	size_ -= len;
	return size_;
}

int64_t OpenBuffer::writeBack(const void* data, size_t len)
{
	if (len == 0)
	{
		return size_;
	}
	size_t newSize = size_ + len;
	int64_t leftCap = cap_ - offset_;
	size_t offset = 0;
	if (leftCap < (int64_t)newSize)
	{
		if (buffer_ && newSize < cap_)
		{
			if (offset_ > 0)
			{
				for (size_t i = 0; i < size_; i++)
				{
					buffer_[i] = buffer_[offset_ + i];
				}
				memset(buffer_ + size_, 0, cap_ + 1 - size_);
			}
			else
			{
				assert(false);
			}
			offset = size_;
		}
		else
		{
			unsigned char* origin = buffer_;
			cap_ = miniCap_;
			while (newSize > cap_)
			{
				cap_ *= 2;
			}
			buffer_ = new unsigned char[cap_ + 2];
			if (!buffer_)
			{
				buffer_ = origin;
				assert(false);
				return -1;
			}
			memset(buffer_, 0, cap_ + 2);
			if (origin)
			{
				if (size_ > 0)
				{
					memcpy(buffer_, origin + offset_, size_);
				}
				delete origin;
				offset = size_;
			}
		}
		offset_ = 0;
	}
	else
	{
		offset = offset_ + size_;
	}
	if (data)
	{
		memcpy(buffer_ + offset, data, len);
	}
	size_ += len;
	return size_;
}

int64_t OpenBuffer::writeString(const std::string& data)
{ 
	return writeBack(data.data(), data.size());
}

int64_t OpenBuffer::writeVInt32(const uint32_t& n)
{
	uint8_t p[10] = { 0 };
	while (true)
	{
		if (n < 0x80) {
			p[0] = (uint8_t)n;
			break;
		}
		p[0] = (uint8_t)(n | 0x80);
		if (n < 0x4000) {
			p[1] = (uint8_t)(n >> 7);
			break;
		}
		p[1] = (uint8_t)((n >> 7) | 0x80);
		if (n < 0x200000) {
			p[2] = (uint8_t)(n >> 14);
			break;
		}
		p[2] = (uint8_t)((n >> 14) | 0x80);
		if (n < 0x10000000) {
			p[3] = (uint8_t)(n >> 21);
			break;
		}
		p[3] = (uint8_t)((n >> 21) | 0x80);
		p[4] = (uint8_t)(n >> 28);
		break;
	}
	return writeBack(&p, strlen((const char*)p));
}

int64_t OpenBuffer::writeVInt64(const uint64_t& n)
{
	if ((n & 0xffffffff) == n) {
		return writeVInt32((uint32_t)n);
	}
	uint8_t p[10] = { 0 };
	uint64_t num = n;
	int64_t i = 0;
	do {
		p[i] = (uint8_t)(num | 0x80);
		num >>= 7;
		++i;
	} while (num >= 0x80);
	p[i] = (uint8_t)num;
	return writeBack(&p, strlen((const char*)p));
}

int64_t OpenBuffer::readVInt64(uint64_t& n)
{
	if (size_ <= 0) return -1;
	if (size_ >= 1)
	{
		unsigned char* p = buffer_ + offset_;
		if (!(p[0] & 0x80))
		{
			n = p[0];
			offset_ += 1;
			size_ -= 1;
			return size_;
		}
		if (size_ <= 1) return -1;
	}
	unsigned char* p = buffer_ + offset_;
	uint32_t r = p[0] & 0x7f;
	for (int i = 1; i < 10; i++)
	{
		r |= ((p[i] & 0x7f) << (7 * i));
		if (!(p[i] & 0x80))
		{
			n = r;
			++i;
			offset_ += i;
			size_ -= i;
			return size_;
		}
	}
	return -1;
}


int64_t OpenBuffer::readString(std::string& data, size_t len)
{
	data.resize(len);
	return readFront((void*)data.data(), data.size());
};


int64_t OpenBuffer::writeInt8(uint8_t c)
{
	return writeBack(&c, 1);
}

int64_t OpenBuffer::readInt8(uint8_t& c)
{
	return readFront(&c, 1);
}

int64_t OpenBuffer::writeInt16(uint16_t w)
{
	char p[2] = { 0 };
#if DATA_BIG_ENDIAN
	* (unsigned char*)(p + 0) = (w & 255);
	*(unsigned char*)(p + 1) = (w >> 8);
#else
	* (unsigned short*)(p) = w;
#endif
	return writeBack(&p, sizeof(p));
}

int64_t OpenBuffer::readInt16(uint16_t& w)
{
	char p[2] = {0};
	int64_t ret = readFront(p, sizeof(p));
	if (ret >= 0)
	{
#if DATA_BIG_ENDIAN
		w = *(const unsigned char*)(p + 1);
		w = *(const unsigned char*)(p + 0) + (w << 8);
#else
		w = *(const unsigned short*)p;
#endif
	}
	return ret;
}

int64_t OpenBuffer::writeInt32(uint32_t l)
{
	char p[4] = { 0 };
#if DATA_BIG_ENDIAN
	* (unsigned char*)(p + 0) = (unsigned char)((l >> 0) & 0xff);
	*(unsigned char*)(p + 1) = (unsigned char)((l >> 8) & 0xff);
	*(unsigned char*)(p + 2) = (unsigned char)((l >> 16) & 0xff);
	*(unsigned char*)(p + 3) = (unsigned char)((l >> 24) & 0xff);
#else
	* (uint32_t*)p = l;
#endif
	return writeBack(&p, sizeof(p));
}

int64_t OpenBuffer::readInt32(uint32_t& l)
{
	char p[4] = { 0 };
	int64_t ret = readFront(p, sizeof(p));
	if (ret >= 0)
	{
#if DATA_BIG_ENDIAN
		l = *(const unsigned char*)(p + 3);
		l = *(const unsigned char*)(p + 2) + (l << 8);
		l = *(const unsigned char*)(p + 1) + (l << 8);
		l = *(const unsigned char*)(p + 0) + (l << 8);
#else 
		l = *(const uint32_t*)p;
#endif
	}
	return ret;
}

int64_t OpenBuffer::writeInt64(uint64_t l)
{
	char p[8] = { 0 };
#if DATA_BIG_ENDIAN
	* (unsigned char*)(p + 0) = (unsigned char)((l >> 0) & 0xff);
	*(unsigned char*)(p + 1) = (unsigned char)((l >> 8) & 0xff);
	*(unsigned char*)(p + 2) = (unsigned char)((l >> 16) & 0xff);
	*(unsigned char*)(p + 3) = (unsigned char)((l >> 24) & 0xff);
	*(unsigned char*)(p + 0) = (unsigned char)((l >> 32) & 0xff);
	*(unsigned char*)(p + 1) = (unsigned char)((l >> 40) & 0xff);
	*(unsigned char*)(p + 2) = (unsigned char)((l >> 48) & 0xff);
	*(unsigned char*)(p + 3) = (unsigned char)((l >> 56) & 0xff);
#else
	* (uint64_t*)p = l;
#endif
	return writeBack(&p, sizeof(p));
}

int64_t OpenBuffer::readInt64(uint64_t& l)
{
	char p[8] = { 0 };
	int64_t ret = readFront(p, sizeof(p));
	if (ret >= 0)
	{
#if DATA_BIG_ENDIAN
		l = *(const unsigned char*)(p + 7);
		l = *(const unsigned char*)(p + 6) + (l << 8);
		l = *(const unsigned char*)(p + 5) + (l << 8);
		l = *(const unsigned char*)(p + 4) + (l << 8);
		l = *(const unsigned char*)(p + 3) + (l << 8);
		l = *(const unsigned char*)(p + 2) + (l << 8);
		l = *(const unsigned char*)(p + 1) + (l << 8);
		l = *(const unsigned char*)(p + 0) + (l << 8);
#else 
		l = *(const uint64_t*)p;
#endif
	}
	return ret;
}


uint16_t OpenBuffer::WriteInt16(uint16_t src)
{
#if DATA_BIG_ENDIAN
	uint16_t dst = 0;
	uint8_t* p = (uint8_t*)&dst;
	*(p + 0) = (uint8_t)(src & 255);
	*(p + 1) = (uint8_t)(src >> 8);
	return dst;
#else
	return src;
#endif
}

uint16_t OpenBuffer::ReadInt16(uint16_t dst)
{
#if DATA_BIG_ENDIAN
	uint16_t src = 0;
	uint8_t* p = (uint8_t*)&dst;
	src = *(p + 1);
	src = *(p + 0) + (src << 8);
	return src;
#else
	return dst;
#endif
}

uint32_t OpenBuffer::WriteInt32(uint32_t src)
{
#if DATA_BIG_ENDIAN
	uint32_t dst = 0;
	uint8_t* p = (uint8_t*)&dst;
	*(p + 0) = (uint8_t)((src >> 0) & 0xff);
	*(p + 1) = (uint8_t)((src >> 8) & 0xff);
	*(p + 2) = (uint8_t)((src >> 16) & 0xff);
	*(p + 3) = (uint8_t)((src >> 24) & 0xff);
	return dst;
#else
	return src;
#endif
}

uint32_t OpenBuffer::ReadInt32(uint32_t dst)
{
#if DATA_BIG_ENDIAN
	uint32_t src = 0;
	uint8_t* p = (uint8_t*)&dst;
	src = *(p + 3);
	src = *(p + 2) + (src << 8);
	src = *(p + 1) + (src << 8);
	src = *(p + 0) + (src << 8);
	return src;
#else 
	return dst;
#endif
}

uint64_t OpenBuffer::WriteInt64(uint64_t src)
{
#if DATA_BIG_ENDIAN
	uint32_t dst = 0;
	uint8_t* p = (uint8_t*)&dst;
	*(p + 0) = (uint8_t)((src >> 0) & 0xff);
	*(p + 1) = (uint8_t)((src >> 8) & 0xff);
	*(p + 2) = (uint8_t)((src >> 16) & 0xff);
	*(p + 3) = (uint8_t)((src >> 24) & 0xff);
	*(p + 0) = (uint8_t)((src >> 32) & 0xff);
	*(p + 1) = (uint8_t)((src >> 40) & 0xff);
	*(p + 2) = (uint8_t)((src >> 48) & 0xff);
	*(p + 3) = (uint8_t)((src >> 56) & 0xff);
	return dst;
#else
	return src;
#endif
}

uint64_t OpenBuffer::ReadInt64(uint64_t dst)
{
#if DATA_BIG_ENDIAN
	uint32_t src = 0;
	uint8_t* p = (uint8_t*)&dst;
	src = *(p + 7);
	src = *(p + 6) + (src << 8);
	src = *(p + 5) + (src << 8);
	src = *(p + 4) + (src << 8);
	src = *(p + 3) + (src << 8);
	src = *(p + 2) + (src << 8);
	src = *(p + 1) + (src << 8);
	src = *(p + 0) + (src << 8);
#else 
	return dst;
#endif
}


//////////OpenSlice//////////////////////
OpenSlice::OpenSlice() 
	:data_(0), 
	size_(0), 
	offset_(0), 
	cap_(0) 
{
}

OpenSlice::OpenSlice(unsigned char* data, size_t size) 
	:data_(data), 
	size_(size), 
	offset_(0), 
	cap_(size) 
{
}

void OpenSlice::setData(unsigned char* data, size_t size)
{
	size_ = size;
	data_ = data;
	offset_ = 0;
	cap_ = size;
}

size_t OpenSlice::size() 
{ 
	return size_; 
}

unsigned char* OpenSlice::data()
{
	if (!data_) return 0;
	if (offset_ >= cap_)
	{
		assert(false);
		return 0;
	}
	return data_ + offset_;
}

int64_t OpenSlice::readString(std::string& data, size_t len)
{
	data.resize(len);
	return readFront((void*)data.data(), data.size());
};	

int64_t OpenSlice::readInt8(uint8_t& c)
{
	return readFront(&c, 1);
}

int64_t OpenSlice::readInt16(uint16_t& w)
{
	char p[2] = { 0 };
	int64_t ret = readFront(p, sizeof(p));
	if (ret >= 0)
	{
#if DATA_BIG_ENDIAN
		w = *(const unsigned char*)(p + 1);
		w = *(const unsigned char*)(p + 0) + (w << 8);
#else
		w = *(const unsigned short*)p;
#endif
	}
	return ret;
}

int64_t OpenSlice::readInt32(uint32_t& l)
{
	char p[4] = { 0 };
	int64_t ret = readFront(p, sizeof(p));
	if (ret >= 0)
	{
#if DATA_BIG_ENDIAN
		l = *(const unsigned char*)(p + 3);
		l = *(const unsigned char*)(p + 2) + (l << 8);
		l = *(const unsigned char*)(p + 1) + (l << 8);
		l = *(const unsigned char*)(p + 0) + (l << 8);
#else 
		l = *(const uint32_t*)p;
#endif
	}
	return ret;
}

int64_t OpenSlice::readInt64(uint64_t& l)
{
	char p[8] = { 0 };
	int64_t ret = readFront(p, sizeof(p));
	if (ret >= 0)
	{
#if DATA_BIG_ENDIAN
		l = *(const unsigned char*)(p + 7);
		l = *(const unsigned char*)(p + 6) + (l << 8);
		l = *(const unsigned char*)(p + 5) + (l << 8);
		l = *(const unsigned char*)(p + 4) + (l << 8);
		l = *(const unsigned char*)(p + 3) + (l << 8);
		l = *(const unsigned char*)(p + 2) + (l << 8);
		l = *(const unsigned char*)(p + 1) + (l << 8);
		l = *(const unsigned char*)(p + 0) + (l << 8);
#else 
		l = *(const uint64_t*)p;
#endif
	}
	return ret;
}

int64_t OpenSlice::readFront(void* data, size_t len)
{
	if (size_ < len) return -1;
	if (!data_)
	{
		assert(false);
		return -1;
	}
	if (data)
	{
		memcpy(data, data_ + offset_, len);
	}
	offset_ += len;
	size_ -= len;
	return size_;
}

int64_t OpenSlice::readBack(void* data, size_t len)
{
	if (size_ < len) return -1;
	if (!data_)
	{
		assert(false);
		return -1;
	}
	if (data)
	{
		memcpy(data, data_ + offset_ + size_ - len, len);
	}
	size_ -= len;
	return size_;
}

};