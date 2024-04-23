#pragma once

#include "../common.h"

class XNet
{
public:
	std::string GetOSUserName();
	static bool GetLocalIps(std::vector<std::string>& vectIps);
};