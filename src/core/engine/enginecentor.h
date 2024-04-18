/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/

#pragma once

#include <vector>
#include <thread>

class XEngine;
class XEngineCentor
{
public:
	XEngineCentor();
	virtual ~XEngineCentor();
	static XEngineCentor& GetInstance();

	void run();
	void wait();
	XEngine* createEngine();
	XEngine* getEngine(int engineId);

	bool checkThread();
protected:
	int makeUid_;
	volatile bool isLockCreate_;
	std::thread::id runThreadId_;
	std::vector<XEngine*> vectEngines_;

	static XEngineCentor* Instance_;
};