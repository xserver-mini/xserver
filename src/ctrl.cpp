#define _CRT_SECURE_NO_WARNINGS

#include "ctrl.h"
#include "utils/app.h"
#include "core/utils/config.h"

static void OnMainThreadEvent(const XEvent& event)
{
	auto eventId = event.getEventId();
	XINFO("[%s]eventId = %d", event.getEventName(), eventId);
	if (XEventAppStart::EEventID == eventId)
	{
//		auto action = XConfig::GetInstance().getValueInt("default", "action", 0);
//		if (!action)
//		{
//			XINFO("==================>>Receiver");
//			return;
//		}
		//XINFO("=================>>Sender");
	}
	else
	{
		XINFO("=================>>");
	}
}


Ctrl* Ctrl::Instance_ = 0;

Ctrl& Ctrl::GetInstance()
{
	if (!Instance_)
		Instance_ = new Ctrl;
	return *Instance_;
}

Ctrl::Ctrl()
{
}

Ctrl::~Ctrl()
{
}

void Ctrl::start()
{
	//XConfig::GetInstance().load("./config.ini");

	std::string content = "[default]\n"
						  "console=2\n"
						  "test=1\n"
						  "action=1\n"
						  "\n"
						  "[UDPPort]\n"
						  "port=51688\n"
						  "heartbeat=3000";
	XConfig::GetInstance().loadText(content);

	XApp::GetInstance().setHandle(OnMainThreadEvent);
	XApp::GetInstance().startAsync();
}

bool Ctrl::sendEvent(XEvent* event, int dstId)
{
	XASSERT(event);
	return XApp::GetInstance().sendEvent(event, dstId);
}
