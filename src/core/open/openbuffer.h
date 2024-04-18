/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/

#ifndef HEADER_OPEN_BUFFER_H
#define HEADER_OPEN_BUFFER_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <string>

namespace open
{ 

class OpenBuffer 
{
private:
	size_t size_;
	size_t offset_;
	size_t cap_;
	size_t miniCap_;
	unsigned char* buffer_;
public:
	OpenBuffer();
	OpenBuffer(size_t capacity);
	~OpenBuffer();

	void clear();
	inline size_t cap() { return cap_; }
	inline size_t size() { return size_; }
	const char* data();
	const char* clearResize(size_t size);
	inline void setCap(size_t capacity) { miniCap_ = capacity; }
	void setData(const char* data, size_t len);
	int64_t writeString(const std::string& data);
	int64_t readString(std::string& data, size_t len);
	inline int64_t writeData(const void* data, size_t len)
	{
		return writeBack(data, len);
	}
	int64_t writeInt8(uint8_t c);
	int64_t readInt8(uint8_t& c);
	int64_t writeInt16(uint16_t w);
	int64_t readInt16(uint16_t& w);
	int64_t writeInt32(uint32_t l);
	int64_t readInt32(uint32_t& l);
	int64_t writeInt64(uint64_t l);
	int64_t readInt64(uint64_t& l);

	int64_t writeVInt32(const uint32_t& n);
	int64_t writeVInt64(const uint64_t& n);
	int64_t readVInt64(uint64_t& n);

	static uint16_t WriteInt16(uint16_t src);
	static uint16_t ReadInt16(uint16_t dst);
	static uint32_t WriteInt32(uint32_t src);
	static uint32_t ReadInt32(uint32_t dst);
	uint64_t WriteInt64(uint64_t src);
	uint64_t ReadInt64(uint64_t dst);

protected:
	int64_t writeBack(const void* data, size_t len);
	int64_t readBack(void* data, size_t len);
	int64_t readFront(void* data, size_t len);
};


class OpenSlice
{
public:
	size_t cap_;
	size_t size_;
	size_t offset_;
	unsigned char* data_;
public:
	OpenSlice();
	OpenSlice(unsigned char* data, size_t size);
	void setData(unsigned char* data, size_t size);
	size_t size();
	unsigned char* data();

	int64_t readString(std::string& data, size_t len);

	int64_t readInt8(uint8_t& c);
	int64_t readInt16(uint16_t& w);
	int64_t readInt32(uint32_t& l);
	int64_t readInt64(uint64_t& l);
	int64_t readData(const void* data, size_t len)
	{
		return readFront((void*)data, len);
	}
protected:
	int64_t readFront(void* data, size_t len);
	int64_t readBack(void* data, size_t len);
};

};

#endif //HEADER_OPEN_BUFFER_H