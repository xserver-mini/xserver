/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/


#pragma once

#include <cassert>
#include <array>
#include <string>
#include <memory>
#include <unordered_map>

const int EXConfigObjectMax = 16;
class XConfig
{
    XConfig();
    XConfig(XConfig&){assert(false);}
    XConfig(const XConfig&){assert(false);}
public:
    class XConfigObject
    {
        std::string name_;
        std::unordered_map<std::string, std::string> keyValues_;
    public:
        const std::string& getName() const;
        bool isExistKey(const std::string& key) const;
        const std::string& getValue(const std::string& key) const;

        friend class XConfig;
    };

    virtual ~XConfig();
    static XConfig& GetInstance();

    void load(const std::string& filePath);
    void loadText(const std::string& content);

    std::shared_ptr<const XConfigObject> getObject(const std::string& name = "default");
    std::string getValue(const std::string& name, const std::string& key, const char* def = "");
    int getValueInt(const std::string& name, const std::string& key, const int def = 0);

protected:
    std::string cfgFilePath_;
    std::array<std::shared_ptr<const XConfigObject>, EXConfigObjectMax> configObjects_;

};

