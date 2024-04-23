cmake_minimum_required(VERSION 3.0)
project(XServer)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

SET(X_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/)
SET(X_SRC_PATH ${X_ROOT}/src)
SET(X_EXTERNAL_PATH ${X_ROOT}external)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    MESSAGE(STATUS "current platform: Linux")

elseif(CMAKE_SYSTEM_NAME MATCHES "Windows")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /utf-8")
    link_libraries(ws2_32)

    MESSAGE(STATUS "current platform: Windows")
elseif(CMAKE_SYSTEM_NAME MATCHES "Android")

    MESSAGE(STATUS "current platform: Android") 
elseif(CMAKE_SYSTEM_NAME MATCHES "Darwin")

    MESSAGE(STATUS "current platform: Apple")  
else()  
    MESSAGE(STATUS "other platform: ${CMAKE_SYSTEM_NAME}")  
endif()


function(auto_source_group source_dir)
    file(GLOB source_files ${source_dir}/*.h ${source_dir}/*.c ${source_dir}/*.cpp)
    foreach(source_file ${source_files})
        file(RELATIVE_PATH relative_path ${X_SRC_PATH} ${source_file})
        get_filename_component(group_name ${relative_path} DIRECTORY)
        string(REPLACE "\\" "/" group_name ${group_name})
        source_group(${group_name} FILES ${source_file})
    endforeach()

    file(GLOB sub_dirs RELATIVE ${source_dir} ${source_dir}/*)
    foreach(sub_dir ${sub_dirs})
        if(IS_DIRECTORY ${source_dir}/${sub_dir})
            auto_source_group(${source_dir}/${sub_dir})
        endif()
    endforeach()
endfunction()

include_directories(${X_SRC_PATH})

include(${X_SRC_PATH}/service/CMakeLists.txt)
include(${X_SRC_PATH}/utils/CMakeLists.txt)
include(${X_SRC_PATH}/core/CMakeLists.txt)

add_executable(${PROJECT_NAME} 
	${X_SRC_PATH}/main.cpp
    ${X_SRC_PATH}/ctrl.h
    ${X_SRC_PATH}/ctrl.cpp

    ${X_CORE_SRC}
	${X_SERVICE_SRC}
	${X_UTILS_SRC}
)

option(OPENSSL_OPTION "OpenSSL opiton" OFF)

if(${OPENSSL_OPTION})

find_package(OpenSSL REQUIRED)
set(OPENSSL_USE_STATIC_LIBS TRUE)
if(OPENSSL_FOUND)
    message(STATUS "Found OpenSSL version: ${OPENSSL_VERSION}")
    add_definitions(-DUSE_OPEN_SSL)
    target_link_libraries(${PROJECT_NAME} ${OPENSSL_LIBRARIES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${OPENSSL_INCLUDE_DIR})
else()
    message(WARNING "OpenSSL not found, some features may be disabled")
endif()

endif()
