/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/


#include "../utils/common.h"

class XRobot;
class XEngine;
class XEventSocket;
class XSocketServer
{
public:
	XSocketServer(int serverId, const std::string& listenName, const std::string& acceptName, int engineNum, int robotNum);
	virtual ~XSocketServer();

	void start();
	bool hasRobotId(int robotId);
	int getRobotId(int port) const;
	bool onEventSocket(XEventSocket* event);

	const int serverId_;
	const int minRobotId_;
	const int maxRobotId_;
	const int engineNum_;
	const int robotNum_;
protected:
	std::string listenName_;
	std::string acceptName_;
	XRobot* listenRobot_;
	std::vector<XEngine*> vectEngine_;
	std::vector<XRobot*> vectRobot_;
	std::array<XRobot*, 0x10000> acceptRobots_;
};
