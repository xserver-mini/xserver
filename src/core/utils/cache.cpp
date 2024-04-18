/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/

#include "cache.h"
#include "common.h"
#include "../open/openfile.h"

XCache::XCache(int cellSize)
	:cellSize_(cellSize)
{
}

XCache::~XCache()
{
}

bool XCache::load(const std::string& fileName)
{
	vectCells_.clear();
	if (cellSize_ <= 0)
	{
		assert(false);
		return false;
	}
	auto cachePath = OpenFile::GetWriteDirPath();
	cachePath = OpenFile::JoinPath(cachePath, "cache");
	auto ret = OpenFile::MakeDir(cachePath);
	if (!ret)
	{
		XERROR("mkdir failed.%s", cachePath.data());
		return false;
	}
	cachePath = OpenFile::JoinPath(cachePath, fileName);
	std::string buffer;
	OpenFile::ReadFile(cachePath, buffer);
	if (buffer.empty())
	{
		return false;
	}
	if (buffer.size() % cellSize_ != 0)
	{
		return false;
	}
	auto count = buffer.size() / cellSize_;
	vectCells_.resize(count);
	for (auto i = 0; i < count; ++i)
	{
		vectCells_[i].append(buffer.data() + i * cellSize_, cellSize_);
	}
	return true;
}

bool XCache::save(const std::string& fileName)
{
	if (cellSize_ <= 0)
	{
		assert(false);
		return false;
	}
	auto cachePath = OpenFile::GetWriteDirPath();
	cachePath = OpenFile::JoinPath(cachePath, "cache");
	auto ret = OpenFile::MakeDir(cachePath);
	if (!ret)
	{
		XERROR("mkdir failed.%s", cachePath.data());
		return false;
	}
	cachePath = OpenFile::JoinPath(cachePath, fileName);
	std::string buffer;
	for (auto& data : vectCells_)
	{
		if (data.size() != cellSize_)
		{
			assert(false);
			return false;
		}
		buffer.append(data.data(), data.size());
	}
	assert(buffer.size() % cellSize_ == 0);
	OpenFile::WriteFile(cachePath, buffer);
	return true;
}


