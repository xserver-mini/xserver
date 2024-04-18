
/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/

#define _CRT_SECURE_NO_WARNINGS

#include "config.h"
#include <iostream>
#include <vector>

static int ReadFile(const std::string& filePath, std::string& buffer, const char* m)
{
    FILE* f = fopen(filePath.c_str(), m);
    if (!f) return -1;
    fseek(f, 0, SEEK_END);
    int len = ftell(f);
    fseek(f, 0, SEEK_SET);
    buffer.resize(len);
    int ret = (int)fread((void*)buffer.data(), 1, len, f);
    fclose(f);
    return ret == 0 ? -1 : len;
}


const std::string& XConfig::XConfigObject::getName() const
{
    return name_;
}

bool XConfig::XConfigObject::isExistKey(const std::string& key) const
{
    auto iter = keyValues_.find(key);
    if(iter == keyValues_.end())
    {
        return false;
    }
    return true;
}

const std::string& XConfig::XConfigObject::getValue(const std::string& key) const
{
    auto iter = keyValues_.find(key);
    if(iter == keyValues_.end())
    {
        static const std::string empty;
        return empty;
    }
    return iter->second;
}


static XConfig* instance_ = nullptr;
XConfig& XConfig::GetInstance()
{
    if(!instance_)
    {
        instance_ = new XConfig;
    }
    return *instance_;
}

XConfig::XConfig()
{
}

XConfig::~XConfig()
{
}

void XConfig::load(const std::string& filePath)
{
    cfgFilePath_ = filePath;
    std::string fileData;
    if (ReadFile(filePath, fileData, "rb") <= 0)
    {
        std::cout << "XConfig::load Failed to open the file for reading." << filePath;
        return;
    }
    loadText(fileData);
}

void XConfig::loadText(const std::string& fileData)
{
    configObjects_.fill(std::shared_ptr<const XConfigObject>());
    std::vector<std::string> vectLines;
    vectLines.resize(vectLines.size() + 1);
    for (size_t i = 0; i < fileData.size(); i++)
    {
        if (fileData[i] == '\n')
        {
            vectLines.resize(vectLines.size() + 1);
            continue;
        }
        if (fileData[i] == '\r')
        {
            continue;
        }
        vectLines.back().push_back(fileData[i]);
    }

    int objectIdx = 0;
    std::string stdLine;
    bool isKey = false;
    std::string key;
    std::string value;
    XConfigObject* object = nullptr;
    for(int i = 0; i < vectLines.size(); ++i)
    {
        auto& line = vectLines[i];
        while (!line.empty() && line[0] == ' ') line.erase(line.begin());
        while (!line.empty() && line.back() == ' ') line.pop_back();
        if(line.size() == 0) continue;
        if(line[0] == '#') continue;
        if(line.size() < 2) continue;

        if(line[0] == '[')
        {
            if(line[line.size() - 1] != ']')
            {
                std::cout << "TSConfig::load erro object name:" << line << std::endl;
                assert(false);
                return;
            }
            if(object)
            {
                configObjects_[objectIdx++] = std::shared_ptr<const XConfigObject>(object);
            }
            if(objectIdx >= EXConfigObjectMax)
            {
                std::cout << "TSConfig::load Too most object. please give a big number for ETSConfigObjectMax";
                assert(false);
                return;
            }
            line.erase(line.begin());
            line.pop_back();
            object = new XConfigObject;
            object->name_ = line;
            continue;
        }

        if(!object)
        {
            std::cout << "TSConfig::load Failed load object name :" << line << std::endl;
            assert(false);
            continue;
        }
        key.clear();
        value.clear();
        isKey = true;
        for (int i = 0; i < line.size(); ++i)
        {
            if (line[i] == '=')
            {
                isKey = false;
                continue;
            }
            if (isKey)
                key.push_back(line[i]);
            else
                value.push_back(line[i]);
        }

        while (!key.empty() && key[0] == ' ') key.erase(key.begin());
        while (!key.empty() && key.back() == ' ') key.pop_back();
        assert(!key.empty());

        while (!value.empty() && value[0] == ' ') value.erase(value.begin());
        while (!value.empty() && value.back() == ' ') value.pop_back();

        object->keyValues_[key] = value;
    }
    if(object)
    {
        configObjects_[objectIdx++] = std::shared_ptr<const XConfigObject>(object);
    }

    for(int i = 0; i < objectIdx; ++i)
    {
        if(configObjects_[i])
            std::cout << "XConfig::load:" << configObjects_[i]->getName().data() << std::endl;
    }
    std::cout << "XConfig::load success." << cfgFilePath_ << std::endl;
}

std::shared_ptr<const XConfig::XConfigObject> XConfig::getObject(const std::string& name)
{
    std::shared_ptr<const XConfig::XConfigObject> ret;
    for(int i = 0; i < configObjects_.size(); ++i)
    {
        ret = configObjects_[i];
        if(!ret) break;
        if(ret->getName().compare(name) == 0)
        {
            break;
        }
        ret.reset();
    }
    return ret;
}

std::string XConfig::getValue(const std::string& name, const std::string& key, const char* def)
{
    std::shared_ptr<const XConfig::XConfigObject> obj = getObject(name.empty() ? "default" : name);
    if(obj && obj->isExistKey(key))
    {
        return obj->getValue(key);
    }
    return def;
}

int XConfig::getValueInt(const std::string& name, const std::string& key, const int def)
{
    std::shared_ptr<const XConfig::XConfigObject> obj = getObject(name.empty() ? "default" : name);
    if (obj && obj->isExistKey(key))
    {
        auto ret = obj->getValue(key);
        if (ret.empty())
        {
            return def;
        }
        return std::atoi(ret.data());
    }
    return def;
}
