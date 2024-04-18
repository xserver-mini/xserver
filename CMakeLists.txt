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
    MESSAGE(STATUS "current source_dir: ${source_dir}")

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

include(src/service/CMakeLists.txt)
include(src/utils/CMakeLists.txt)
include(src/core/CMakeLists.txt)

add_executable(${PROJECT_NAME} 
	src/main.cpp
    src/ctrl.h
    src/ctrl.cpp

    ${X_CORE_SRC}
	${X_SERVICE_SRC}
	${X_UTILS_SRC}
)

message(STATUS "")
message(STATUS "        ---( ${PROJECT_NAME} " ${EVENT_VERSION} " )---")
message(STATUS "")
message(STATUS "Available event backends: ${BACKENDS}")
message(STATUS "CMAKE_BINARY_DIR:         ${CMAKE_BINARY_DIR}")
message(STATUS "CMAKE_CURRENT_BINARY_DIR: ${CMAKE_CURRENT_BINARY_DIR}")
message(STATUS "CMAKE_SOURCE_DIR:         ${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "PROJECT_BINARY_DIR:       ${PROJECT_BINARY_DIR}")
message(STATUS "PROJECT_SOURCE_DIR:       ${PROJECT_SOURCE_DIR}")
message(STATUS "CMAKE_MODULE_PATH:        ${CMAKE_MODULE_PATH}")
message(STATUS "CMAKE_COMMAND:            ${CMAKE_COMMAND}")
message(STATUS "CMAKE_ROOT:               ${CMAKE_ROOT}")
message(STATUS "CMAKE_SYSTEM:             ${CMAKE_SYSTEM}")
message(STATUS "CMAKE_SYSTEM_NAME:        ${CMAKE_SYSTEM_NAME}")
message(STATUS "CMAKE_SYSTEM_VERSION:     ${CMAKE_SYSTEM_VERSION}")
message(STATUS "CMAKE_SYSTEM_PROCESSOR:   ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "CMAKE_SKIP_RPATH:         ${CMAKE_SKIP_RPATH}")
message(STATUS "CMAKE_SKIP_INSTALL_RPATH: ${CMAKE_SKIP_INSTALL_RPATH}")
message(STATUS "CMAKE_INSTALL_RPATH:      ${CMAKE_INSTALL_RPATH}")
message(STATUS "CMAKE_VERBOSE_MAKEFILE:   ${CMAKE_VERBOSE_MAKEFILE}")
message(STATUS "CMAKE_C_FLAGS:            ${CMAKE_C_FLAGS}")
message(STATUS "CMAKE_BUILD_TYPE:         ${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_C_COMPILER:         ${CMAKE_C_COMPILER} (id ${CMAKE_C_COMPILER_ID}, clang ${CLANG}, GNUC ${GNUC}, version ${CMAKE_C_COMPILER_VERSION})")
message(STATUS "CMAKE_AR:                 ${CMAKE_AR}")
message(STATUS "CMAKE_RANLIB:             ${CMAKE_RANLIB}")
message(STATUS "CMAKE_INSTALL_PREFIX:     ${CMAKE_INSTALL_PREFIX}")
message(STATUS "CMAKE_DEBUG_POSTFIX:      ${CMAKE_DEBUG_POSTFIX}")
message(STATUS "CMAKE_RUNTIME_OUTPUT_DIRECTORY:   ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "")