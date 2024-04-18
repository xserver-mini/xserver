/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/


#define _CRT_SECURE_NO_WARNINGS
#include "logger.h"
#include <cstdarg>
#include <cstdio>
#include <cassert>
#include <vector>
#include <ctime>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "../open/openfile.h"

#if defined(ANDROID)
#include <android/log.h>
#endif

XLogger* XLogger::Instance_ = nullptr;

XLogger& XLogger::GetInstance()
{
	if (!Instance_)
		Instance_ = new XLogger;
	return *Instance_;
}

void XLogger::Log(const char* file, int line, const char* func, const char* format, ...)
{
    auto data = new XData;
    auto& buffer = data->buffer_;
    int count = 1;
    int maxSize = 0;
    va_list ap;
    for (;;)
    {
        maxSize = count * 256;
        buffer.resize(maxSize + 1, 0);

        va_start(ap, format);
        data->size_ = vsnprintf(buffer.data(), maxSize, format, ap);
        va_end(ap);
        if (data->size_ >= 0 && data->size_ < maxSize)
        {
            break;
        }
        ++count;
    }
    buffer.data()[data->size_] = 0;
    GetInstance().print(file, line, func, ELog, data);
}

void XLogger::Info(const char* file, int line, const char* func, const char* format, ...)
{
    auto data = new XData;
    auto& buffer = data->buffer_;
    int count = 1;
    int maxSize = 0;
    va_list ap;
    for (;;)
    {
        maxSize = count * 256;
        buffer.resize(maxSize + 1, 0);

        va_start(ap, format);
        data->size_ = vsnprintf(buffer.data(), maxSize, format, ap);
        va_end(ap);
        if (data->size_ >= 0 && data->size_ < maxSize)
        {
            break;
        }
        ++count;
    }
    buffer.data()[data->size_] = 0;
    GetInstance().print(file, line, func, EInfo, data);
}

void XLogger::Debug(const char* file, int line, const char* func, const char* format, ...)
{
    auto data = new XData;
    auto& buffer = data->buffer_;
    int count = 1;
    int maxSize = 0;
    va_list ap;
    for (;;)
    {
        maxSize = count * 256;
        buffer.resize(maxSize + 1, 0);

        va_start(ap, format);
        data->size_ = vsnprintf(buffer.data(), maxSize, format, ap);
        va_end(ap);
        if (data->size_ >= 0 && data->size_ < maxSize)
        {
            break;
        }
        ++count;
    }
    buffer.data()[data->size_] = 0;
    GetInstance().print(file, line, func, EDebug, data);
}

void XLogger::Warn(const char* file, int line, const char* func, const char* format, ...)
{
    auto data = new XData;
    auto& buffer = data->buffer_;
    int count = 1;
    int maxSize = 0;
    va_list ap;
    for (;;)
    {
        maxSize = count * 256;
        buffer.resize(maxSize + 1, 0);

        va_start(ap, format);
        data->size_ = vsnprintf(buffer.data(), maxSize, format, ap);
        va_end(ap);
        if (data->size_ >= 0 && data->size_ < maxSize)
        {
            break;
        }
        ++count;
    }
    buffer.data()[data->size_] = 0;
    GetInstance().print(file, line, func, EWarn, data);
}

void XLogger::Error(const char* file, int line, const char* func, const char* format, ...)
{
    auto data = new XData;
    auto& buffer = data->buffer_;
    int count = 1;
    int maxSize = 0;
    va_list ap;
    for (;;)
    {
        maxSize = count * 256;
        buffer.resize(maxSize + 1, 0);

        va_start(ap, format);
        data->size_ = vsnprintf(buffer.data(), maxSize, format, ap);
        va_end(ap);
        if (data->size_ >= 0 && data->size_ < maxSize)
        {
            break;
        }
        ++count;
    }
    buffer.data()[data->size_] = 0;
    GetInstance().print(file, line, func, EError, data);
}

XLogger::XLogger()
    :isRunning_(false),
    consoleLog_(1),
    level_(ELog)
{
    std::thread th(XLogger::OnRun, this);
    th.detach();
}

XLogger::~XLogger()
{
    while (!queue_.empty())
    {
        delete queue_.front();
        queue_.pop();
    }
}

void XLogger::setLogDir(const std::string& logDir)
{
    lock_.lock();
    logDir_ = logDir;
    if (!logDir_.empty())
    {
        if (logDir_.back() != '/' && logDir_.back() != '\\')
        {
            logDir_.push_back('/');
        }
    }
    lock_.unlock();
    open::OpenFile::MakeDir(logDir_);
}

static std::string MSecondToStr(int64_t msecond)
{
    std::time_t timestamp = msecond / 1000;
    char buffer[80] = {};
    std::tm* info = std::localtime(&timestamp);
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", info);
    return buffer + std::string(".") +  std::to_string(msecond % 1000);
}
static const std::vector<std::string> VectLabels = { "[LOGG]", "[INFO]", "[DEBU]", "[WARN]", "[ERRO]" , "[ALL]"};
static const std::vector<std::string> VectFiles  = { "log", "info", "debug", "warn", "error", "all"};

void XLogger::print(const char* file, int line, const char* func, int level, XData* data)
{
    if (level >= VectLabels.size() || level < 0)
    {
        delete data;
        assert(false);
        return;
    }
    if (!data)
    {
        assert(false);
        return;
    }
    auto duration = std::chrono::system_clock::now().time_since_epoch();
    int64_t msecond = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    data->level_ = level;
    data->line_ = line;
    data->time_ = msecond;
    data->theadId_ = std::this_thread::get_id();
    data->func_ = func;

        //ss << VectLabels[level] << "[" << MSecondToStr(data->time_) << "][0x" << std::hex << data->theadId_ << std::dec << "]["
        //    << data->func_ << ":" << data->line_ << "] " << data->data_ << std::endl;
    if (consoleLog_ == 1)
    {
        std::stringstream ss;
        ss << VectLabels[level] << "[" << MSecondToStr(data->time_) << "][0x" << std::hex << data->theadId_ << std::dec << "]["
            << data->func_ << data->line_ << "] " << data->buffer_.data() << std::endl;
#if defined(ANDROID)
        __android_log_print(ANDROID_LOG_DEBUG, "Speaker", "%s", ss.str().data());
#else
        std::cout << ss.str();
#endif
    }

    lock_.lock();
    queue_.push(data);
    lock_.unlock();
    cv_.notify_one();
}

void XLogger::OnRun(XLogger* logger)
{
    if (!logger)
    {
        assert(false);
        return;
    }
    logger->onRun();
}

void XLogger::onRun()
{
    assert(VectLabels.size() == EMax);
    assert(VectFiles.size() == EMax);

    char buffer[80] = {};
    std::tm* info = 0;
    std::time_t timestamp = 0;
    int64_t currentDayIdx = 0;
    int64_t lastDayIdx = 0;
    isRunning_ = true;
    XData* data = 0;
    bool isEmpty = false;
    while (isRunning_)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        lock_.lock();
        if (queue_.empty())
        {
            lock_.unlock();
            cv_.wait(lock);
            continue;
        }
        lock_.unlock();

        if (!logDir_.empty())
        {
            timestamp = std::time(0);
            info = std::localtime(&timestamp);
            currentDayIdx = (info->tm_year + 1900) * 10000 + (info->tm_mon + 1) * 100 + info->tm_mday;
            if (lastDayIdx != currentDayIdx)
            {
                lastDayIdx = currentDayIdx;
                lock_.lock();
                if (logDir_.empty()) logDir_ = "./";
                lock_.unlock();
                std::string dayDir = logDir_ + std::to_string(currentDayIdx);
                open::OpenFile::MakeDir(dayDir);
                for (int i = 0; i < EMax; ++i)
                {
                    auto& file = files_[i];
                    if (files_[i].is_open()) files_[i].close();
                    logPaths_[i] = dayDir + "/" + VectFiles[i] + ".log";
                    files_[i].open(logPaths_[i], std::ios::binary | std::ios::app);
                }
            }
        }
        while (isRunning_)
        {
            lock_.lock();
            if (queue_.empty())
            {
                lock_.unlock();
                break;
            }
            data = queue_.front();
            queue_.pop();
            lock_.unlock();
            if (!data)
            {
                assert(false);
                continue;
            }
            int level = data->level_;
            assert(level >= 0);
            if(level >= VectLabels.size())
            {
                assert(false);
            }

            std::stringstream ss;
            if (consoleLog_ == 2)
            {
                ss << VectLabels[level] << "[" << MSecondToStr(data->time_) << "][0x" << std::hex << data->theadId_ << std::dec << "]["
                    << data->func_ << data->line_ << "] " << data->buffer_.data() << std::endl;
#if defined(ANDROID)
                __android_log_print(ANDROID_LOG_DEBUG, "Speaker", "%s", ss.str().data());
#else
                std::cout << ss.str();
#endif
            }

            if (!logPaths_[level].empty())
            {
                if (files_[level].is_open())
                {
                    files_[level] << ss.str();
                    files_[level].flush();
                }
                if (files_[EAll].is_open())
                {
                    files_[EAll] << ss.str();
                    files_[EAll].flush();
                }
            }
            delete data;
        }
    }
}

//std::cout << "seconds:" << std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count() << std::endl;
//std::cout << "milliseconds:" << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() << std::endl;
//std::cout << "microseconds:" << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count() << std::endl;
//std::cout << "nanoseconds:" << std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count() << std::endl;
