set(X_SERVICE_SRC)

set(FILE_HPP
	testa/service.h
	testa/handle.h

	testb/service.h
	testb/handle.h

	httpclient/service.h
	httpserver/service.h
	httpserver/handle.h
	httpserver/http_util.h

	tcpserver/service.h
	tcpclient/service.h

	udpdevice/service.h
	udpdevice/device.h
	udpdevice/handle.h
	udpdevice/data.h
	udpdevice/event.h

	udpport/service.h
	udpport/device.h
	udpport/handle.h
	udpport/event.h
	udpport/data.h
)

set(FILE_CPP
	testa/service.cpp
	testb/service.cpp

	httpclient/service.cpp
	httpserver/service.cpp

	tcpserver/service.cpp
	tcpclient/service.cpp

	udpdevice/service.cpp
	udpdevice/device.cpp

	udpport/service.cpp
	udpport/device.cpp
)

set(TMP_SRC ${FILE_HPP} ${FILE_CPP})
foreach(item ${TMP_SRC})
    list(APPEND X_SERVICE_SRC ${X_SRC_PATH}/service/${item})
endforeach(item)

auto_source_group(${X_SRC_PATH}/service)