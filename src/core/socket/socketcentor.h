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
#include <array>
#include <vector>
#include <unordered_map>

class XRobot;
class XEngine;
class XEventSocket;
class XSocketServer;
class XSocketCentor
{
public:
	enum _
	{
		EMaxServerNum = 10,
		EMinSocketRobotId = 10000000
	};
    XSocketCentor();
	virtual ~XSocketCentor();
	static XSocketCentor& GetInstance();

    void start();
	bool isHereRobot(int robotId);
	bool onEventSocket(XEventSocket* event);
	int createServer(const std::string& listenName, const std::string& acceptName, int threadNum);
protected:
    volatile bool isLockCreate_;
	std::vector<XSocketServer*> vectSevers_;
	static XSocketCentor* Instance_;
};