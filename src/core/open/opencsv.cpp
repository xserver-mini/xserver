/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/
#include "opencsv.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <map>

namespace open
{
static bool CheckFilePath(const std::string& filePath);
static int64_t ReadFile(const std::string& filePath, std::string& buffer, const char* m);
static int64_t WriteFile(const std::string& filePath, std::string& buffer, const char* m);

std::string& OpenCSV::CSVLine::operator[](size_t idx)
{
    if (idx >= line_.size())
    {
        line_.resize(idx + 1);
    }
    return line_[idx];
}

std::string& OpenCSV::CSVLine::operator[](const std::string& key)
{
    OpenCSV* csv = dynamic_cast<OpenCSV*>(csv_);
    if (csv == 0 || csv->lines_.empty())
    {
        static std::string emptyStr;
        assert(false);
        return emptyStr;
    }
    CSVLine& keyLine = *csv->lines_[0];
    size_t idx = 0;
    for (; idx < keyLine.size(); ++idx)
    {
        if (keyLine[idx] == key) break;
    }
    if (idx < keyLine.size())
    {
        return line_[idx];
    }
    assert(false);
    csv->emptyStr_.clear();
    return csv->emptyStr_;
}

//OpenCSV
OpenCSV::OpenCSV()
{
    lines_.reserve(1024);
}

OpenCSV::~OpenCSV()
{
    for (size_t i = 0; i < lines_.size(); i++)
    {
        delete lines_[i];
    }
    lines_.clear();
}

OpenCSV::OpenCSV(const List& list)
{
    lines_.reserve(1024);
    if (list.size() > 0)
    {
        auto line = new CSVLine(this);
        line->line() = list;
        lines_.push_back(line);
    }
}

void OpenCSV::operator=(const List& list)
{ 
    if (list.size() > 0)
    {
        auto line = new CSVLine(this);
        line->line() = list;
        lines_.push_back(line);
    }
}

OpenCSV::OpenCSV(const Line& line)
{
    lines_.reserve(1024);
    if (line.size() > 0)
    {
        auto row = new CSVLine(this);
        row->line() = line;
        lines_.push_back(row);
    }
}

void OpenCSV::operator=(const Line& line)
{
    if (line.size() > 0)
    {
        auto row = new CSVLine(this);
        row->line() = line;
        lines_.push_back(row);
    }
}

OpenCSV::CSVLine& OpenCSV::operator[](size_t idx)
{
    if (idx >= lines_.size())
    {
        while (idx >= lines_.size())
        {
            auto row = new CSVLine(this);
            lines_.push_back(row);
        }
    }
    return *lines_[idx];
}

void OpenCSV::operator>>(std::string& output)
{
    std::string str;
    for (auto& line : lines_)
    {
        if (line->empty()) continue;
        for (size_t j = 0; j < line->size(); ++j)
        {
            str.append(j > 0 ? "," + (*line)[j] : (*line)[j]);
        }
        str.append("\n");
    }
    if (!CheckFilePath(output))
    {
        output.append(str);
        return;
    }
    WriteFile(output, str, "wb");
}

void OpenCSV::operator<<(const std::string& input)
{
    std::string buffer;
    if (CheckFilePath(input))
    {
        ReadFile(input, buffer, "rb");
    }
    else
    {
        buffer = input;
    }
    lines_.clear();
    if (buffer.empty()) return;
    size_t row    = 0;
    size_t column = 0;
    for (size_t i = 0; i < buffer.size(); ++i)
    {
        if (buffer[i] == '\n')
        {
            if (!lines_.back()->empty())
            {
                ++row;
            }
            column = 0;
            continue;
        }
        if (buffer[i] == ',')
        {
            ++column;
            continue;
        }
        (*this)[row][column].push_back(buffer[i]);
    }
}

void OpenCSV::sort(const std::string& key)
{
    if (lines_.empty())
    {
        assert(false);
        return;
    }
    CSVLine& keyLine = *lines_[0];
    size_t idx = 0;
    for (; idx < keyLine.size(); ++idx)
    {
        if (keyLine[idx] == key) break;
    }
    if (idx >= keyLine.size())
    {
        assert(false);
        return;
    }
    std::multimap<std::string, CSVLine*> sortMap;
    CSVLine* row = 0;
    for (size_t i = 1; i < lines_.size(); i++)
    {
        row = lines_[i];
        auto& key = (*row)[idx];
        sortMap.insert(std::pair<std::string, CSVLine*>(key, row));
    }
    size_t i = 1;
    for (auto& iter : sortMap)
    {
        lines_[i++] = iter.second;
    }
}

static bool CheckFilePath(const std::string& filePath)
{
    if (!filePath.empty() && filePath.size() < 1024)
    {
        if (!strstr(filePath.c_str(), "\n") && !strstr(filePath.c_str(), ","))
        {
            if (strstr(filePath.c_str(), "/")) return true;
        }
    }
    return false;
}

static int64_t ReadFile(const std::string& filePath, std::string& buffer, const char* m)
{
    FILE* f = 0;
#ifdef _MSC_VER
    fopen_s(&f, filePath.c_str(), m);
#else
    f = fopen(filePath.c_str(), m);
#endif
    if (!f) return -1;
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    buffer.resize(len, 0);
    size_t ret = fread((void*)buffer.data(), 1, len, f);
    fclose(f);
    return ret == 0 ? -1 : len;
}

static int64_t WriteFile(const std::string& filePath, std::string& buffer, const char* m)
{
    FILE* f = 0;
#ifdef _MSC_VER
    fopen_s(&f, filePath.c_str(), m);
#else
    f = fopen(filePath.c_str(), m);
#endif
    if (!f) return -1;
    size_t ret = fwrite((void*)buffer.data(), 1, buffer.size(), f);
    fclose(f);
    return ret == 0 ? -1 : buffer.size();
}

}