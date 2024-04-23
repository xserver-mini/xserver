#include "app.h"
#include "core/robot/robotcentor.h"
#include "core/engine/enginecentor.h"

#include "utils/event.h"

#include "core/service/servicecentor.h"
#include "core/socket/socketcentor.h"

#include "service/testa/service.h"
#include "service/testb/service.h"
#include "service/tcpserver/service.h"
#include "service/tcpclient/service.h"

#include "service/httpclient/service.h"
#include "service/httpserver/service.h"

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
	serviceCentor.registerService<TestA::Service>("TestA");
	serviceCentor.registerService<TestB::Service>("TestB");

	serviceCentor.registerService<TcpClient::Service>("TcpClient");
	serviceCentor.registerService<TcpListen::Service>("TcpListen");
	serviceCentor.registerService<TcpAccept::Service>("TcpAccept");

	serviceCentor.registerService<HttpClient::Service>("HttpClient");
	serviceCentor.registerService<HttpListen::Service>("HttpListen");
	serviceCentor.registerService<HttpAccept::Service>("HttpAccept");

	//start robot;
	auto& robotCentor = XRobotCentor::GetInstance();
	robotCentor.createSingleRobot<XRobot>(ERobotIDTestA, "TestA");
	robotCentor.createSingleRobot<XRobot>(ERobotIDTestB, "TestB");
	robotCentor.createSingleRobot<XRobot>(ERobotIDTcpClient, "TcpClient");

	robotCentor.createSingleRobot<XRobot>(ERobotIDHttpClient, "HttpClient");

	auto& socketCentor = XSocketCentor::GetInstance();
	socketCentor.createServer("TcpListen", "TcpAccept", 2, 8);
	socketCentor.createServer("HttpListen", "HttpAccept", 2, 8);
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