/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/


#pragma once

#include "../common.h"

class XEvent;
class XService;
class XEngine;
class XRobot
{
public:
	XRobot(int robotId);
	virtual ~XRobot();
	
	virtual void onInit();
	virtual void onStart();
	virtual void onStop();

	bool sendEvent(XEvent* event, int targetId = 0);
	void onEvent(const XEvent& event);

	virtual const std::string& serviceName();
	void setServiceName(const std::string& name);
	bool isIdle();
	inline bool isRunning() { return isRunning_; }

	const int robotId_;
	bool isLog_;
	bool isCostTime_;
	int minCostTime_;
	void* server_;
protected:
	XService* service_;
	std::string serviceName_;
	bool isRunning_;
private:
	int engineId_;
	friend class XEngine;
	friend class XRobotCentor;
};