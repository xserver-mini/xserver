/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/


#include <string>
#include <cassert>
#include <iostream>
#include <unordered_map>
#include "service.h"

class XRobot;
class XServiceCentor
{
    typedef XService*(* NewClass)(XRobot*);
public:
    XServiceCentor();
	virtual ~XServiceCentor();
	static XServiceCentor& GetInstance();

    void start();
    XService* createService(XRobot* robot);

    template <class T>
    bool registerService(const std::string& serviceName)
    {
        if (isLockCreate_)
        {
            XERROR("XServiceCentor::registerService serviceName=%s", serviceName.data());
            assert(false);
            return false;
        }

        auto iter = mapClassCreate_.find(serviceName);
        if (iter != mapClassCreate_.end())
        {
            XERROR("XServiceCentor::registerService Service [%s] is exist!", serviceName.data());
            assert(false);
            return false;
        }
        mapClassCreate_[serviceName] = (NewClass)&T::New;
        return true;
    }
protected:
    volatile bool isLockCreate_;
    std::unordered_map<int, XService*> mapServices_;
    std::unordered_map<std::string, NewClass> mapClassCreate_;

	static XServiceCentor* Instance_;
};