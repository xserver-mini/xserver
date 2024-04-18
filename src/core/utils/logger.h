/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/


#pragma once

#include <string>
#include <fstream>
#include <atomic>
#include <queue>
#include <thread>
#include <cassert>
#include <memory>
#include <condition_variable>

#define XLOG(format, ...)   XLogger::Log(__FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)
#define XINFO(format, ...)  XLogger::Info(__FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)
#define XDEBUG(format, ...) XLogger::Debug(__FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)
#define XWARN(format, ...)  XLogger::Warn(__FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)
#define XERROR(format, ...) XLogger::Error(__FILE__, __LINE__, __FUNCTION__, format, ##__VA_ARGS__)
#define XASSERT assert

class XLogger
{
	class Xlock
	{
	public:
		Xlock() :flag_(false) {}
		inline void lock()
		{
			bool expect = false;
			while (!flag_.compare_exchange_weak(expect, true)) { expect = false; }
		}
		inline void unlock() { flag_.store(false); }
	protected:
		std::atomic<bool> flag_;
	};
public:
	enum ELevel
	{
		ELog = 0,
		EInfo,
		EDebug,
		EWarn,
		EError,
		EAll,
		EMax
	};
	struct XData
	{
		int level_;
		int line_;
		int64_t time_;
		//std::string data_;
		int size_;
		std::vector<char> buffer_;
		std::string func_;
		std::thread::id theadId_;
	};

	XLogger();
	virtual ~XLogger();

	static void Log(const char* file, int line, const char* func, const char* format, ...);
	static void Info(const char* file, int line, const char* func, const char* format, ...);
	static void Debug(const char* file, int line, const char* func, const char* format, ...);
	static void Warn(const char* file, int line, const char* func, const char* format, ...);
	static void Error(const char* file, int line, const char* func, const char* format, ...);

	void setLogDir(const std::string& logDir);
	inline void setConsoleLog(int level) { consoleLog_ = level; }

	static XLogger& GetInstance();
protected:
	void print(const char* file, int line, const char* func, int level, XData* data);
	static void OnRun(XLogger* logger);
	void onRun();

	int consoleLog_;
	bool isRunning_;
	ELevel level_;
	std::string logDir_;
	std::string logPaths_[EMax];
	std::ofstream files_[EMax];

	Xlock lock_;
	std::mutex mutex_;
	std::condition_variable cv_;
	std::queue<XData*> queue_;

	static XLogger* Instance_;
};
