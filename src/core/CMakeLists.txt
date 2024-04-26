set(X_CORE_SRC)

set(FILE_HPP
	service/service.h
	service/servicecentor.h

	robot/event.h
	robot/robot.h
	robot/robotcentor.h
	engine/engine.h
	engine/enginecentor.h

	socket/eventsocket.h
	socket/servicesocket.h
	socket/socketcentor.h
	socket/socketserver.h

	timer/servicetimer.h

	open/open.h
	open/opentime.h
	open/opensocket.h
	open/socket_os.h
	open/openbuffer.h
	open/openjson.h
	open/openfile.h
	open/opencsv.h
	open/openstring.h

	utils/spinlock.h
	utils/logger.h
	utils/config.h
	utils/cache.h

	http/openssl.h
	http/httplib.h
	http/serviceclient.h
	http/serviceserver.h

	platform/net.h

	app.h
	common.h
)

set(FILE_CPP
	service/service.cpp
	service/servicecentor.cpp
	robot/event.cpp
	robot/robot.cpp
	robot/robotcentor.cpp
	engine/engine.cpp
	engine/enginecentor.cpp

	socket/eventsocket.cpp
	socket/servicesocket.cpp
	socket/socketcentor.cpp
	socket/socketserver.cpp

	timer/servicetimer.cpp

	open/opentime.cpp
	open/openjson.cpp
	open/opensocket.cpp
	open/socket_os.c
	open/openbuffer.cpp
	open/openjson.cpp
	open/openfile.cpp
	open/opencsv.cpp
	open/openstring.cpp

	utils/logger.cpp
	utils/config.cpp
	utils/cache.cpp

	http/openssl.cpp
	http/httplib.cpp
	http/serviceclient.cpp
	http/serviceserver.cpp

	platform/net.cpp

	app.cpp
)

if(CMAKE_SYSTEM_NAME MATCHES "Windows")

list(APPEND FILE_HPP open/wepoll.h)
list(APPEND FILE_CPP open/wepoll.c)

list(APPEND FILE_HPP platform/utils-win32.h)
list(APPEND FILE_CPP platform/utils-win32.cpp)
list(APPEND FILE_CPP platform/win32.cpp)

elseif(CMAKE_SYSTEM_NAME MATCHES "Android")

list(APPEND FILE_HPP platform/jnihelper.h)
list(APPEND FILE_CPP platform/jnihelper.cpp)
list(APPEND FILE_HPP platform/utf8.h)
list(APPEND FILE_CPP platform/utf8.cpp)
list(APPEND FILE_HPP platform/convertutf.h)
list(APPEND FILE_CPP platform/convertutf.c)
list(APPEND FILE_CPP platform/android.cpp)

elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")

list(APPEND FILE_CPP platform/linux.cpp)

elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")

list(APPEND FILE_CPP platform/linux.cpp)

endif()

set(TMP_SRC ${FILE_HPP} ${FILE_CPP})
foreach(item ${TMP_SRC})
    list(APPEND X_CORE_SRC ${X_SRC_PATH}/core/${item})
endforeach(item)

auto_source_group(${X_SRC_PATH}/core)
