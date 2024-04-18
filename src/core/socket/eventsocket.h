/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/


#pragma once

#include "../open/opensocket.h"
#include "../robot/event.h"

//XEventSocket
class XEventSocket : public XEvent
{
public:
	XEventSocket();
	virtual ~XEventSocket();
	int getEventId() const;
};

//XEventSocketMsg
class XEventSocketMsg : public XEventSocket
{
public:
	enum _
	{
		EEventID = 1
	};
	XEventSocketMsg();
	virtual ~XEventSocketMsg();
	virtual int getEventId() const;
	const open::OpenSocketMsg* socketMsg_;
};

//XEventSocketAccept
class XEventSocketAccept : public XEventSocket
{
public:
	enum _
	{
		EEventID = 2
	};
	XEventSocketAccept();
	virtual ~XEventSocketAccept();
	virtual int getEventId() const;
	std::string ip_;
	int port_;
	int fd_;
};