/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/

#if defined(ANDROID)

#include "net.h"
#include "jnihelper.h"

static const char* javaUtil = "com/linyou/speaker/Speaker";

bool XNet::GetLocalIps(std::vector<std::string>& vectIps)
{
	std::string localIPs = JniHelper::callStaticStringMethod(javaUtil, "GetLocalIPs");
	std::string item;
	for (int i = 0; i < localIPs.size(); ++i)
	{
		auto& c = localIPs[i];
		if (c == '\n')
		{
			if (!item.empty())
			{
				vectIps.push_back(item);
			}
			item.clear();
			continue;
		}
		item.push_back(c);
	}
	if (!item.empty() && item.find(":") == std::string::npos)
	{
		vectIps.push_back(item);
	}
	return true;
}

#endif