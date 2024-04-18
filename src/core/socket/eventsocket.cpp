/***************************************************************************
 * Copyright (C) 2023-, openlinyou, <linyouhappy@foxmail.com>
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 ***************************************************************************/


#include "eventsocket.h"
#include "../utils/common.h"

//XEventSocket
XEventSocket::XEventSocket()
{
	eEventType_ = EXEventTypeSocket;
}

XEventSocket::~XEventSocket()
{
}

int XEventSocket::getEventId() const 
{ 
	XASSERT(false);
	return -1; 
}


//XEventSocketMsg
XEventSocketMsg::XEventSocketMsg()
	:socketMsg_(nullptr)
{
}

XEventSocketMsg::~XEventSocketMsg()
{
	if (socketMsg_)
	{
		delete socketMsg_;
		socketMsg_ = nullptr;
	}
}

int XEventSocketMsg::getEventId() const
{
	return XEventSocketMsg::EEventID;
}


//XEventSocketAccept
XEventSocketAccept::XEventSocketAccept()
{
}

XEventSocketAccept::~XEventSocketAccept()
{
}

int XEventSocketAccept::getEventId() const
{
	return XEventSocketAccept::EEventID;
}