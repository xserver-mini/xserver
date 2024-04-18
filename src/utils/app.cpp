#include "app.h"
#include "core/robot/robotcentor.h"
#include "core/engine/enginecentor.h"
#include "utils/event.h"

#include "core/service/servicecentor.h"
#include "core/socket/socketcentor.h"
#include "core/timer/servicetimer.h"

#include "service/test/service.h"
#include "service/tcpserver/service.h"
#include "service/tcpclient/service.h"

XApp* XApp::CreateApp()
{
	return new App;
}

App::App()
{
}

App::~App()
{
}

void App::start()
{
	auto logPath = XConfig::GetInstance().getValue("default", "LogDir", "./log");
	auto console = XConfig::GetInstance().getValueInt("default", "console", 0);
	XLogger::GetInstance().setLogDir(logPath);
	XLogger::GetInstance().setConsoleLog(console);

	//register task
	auto& serviceCentor = XServiceCentor::GetInstance();
	//name bind Service
	serviceCentor.registerService<Test::Service>("Test");

	serviceCentor.registerService<TcpListen::Service>("TcpListen");
	serviceCentor.registerService<TcpAccept::Service>("TcpAccept");
	serviceCentor.registerService<TcpClient::Service>("TcpClient");

	//start robot;
	auto& robotCentor = XRobotCentor::GetInstance();
	robotCentor.createSingleRobot<XRobot>(ERobotIDTest, "Test");
	robotCentor.createSingleRobot<XRobot>(ERobotIDTcpClient, "TcpClient");


	auto& socketCentor = XSocketCentor::GetInstance();
	socketCentor.createServer("TcpListen", "TcpAccept", 2);
	socketCentor.start();
}

void App::onMainThreadEvent(XEvent* event)
{
	if (handle_ && event)
	{
		handle_(*event);
	}
	else
	{
		XASSERT(false);
	}
}