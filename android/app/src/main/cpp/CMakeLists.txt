# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html.
# For more examples on how to use CMake, see https://github.com/android/ndk-samples.

# Sets the minimum CMake version required for this project.
cmake_minimum_required(VERSION 3.22.1)

# Declares the project name. The project name can be accessed via ${ PROJECT_NAME},
# Since this is the top level CMakeLists.txt, the project name is also accessible
# with ${CMAKE_PROJECT_NAME} (both CMake variables are in-sync within the top level
# build script scope).
project(XServer)

set(CMAKE_VERBOSE_MAKEFILE on)

SET(X_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../)
get_filename_component(X_ROOT ${X_ROOT} ABSOLUTE)
message(STATUS "X_ROOT: ${X_ROOT}")
SET(X_SRC_PATH ${X_ROOT}/src)
SET(X_EXTERNAL_PATH ${X_ROOT}external)

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

include(${X_SRC_PATH}/service/CMakeLists.txt)
include(${X_SRC_PATH}/utils/CMakeLists.txt)
include(${X_SRC_PATH}/core/CMakeLists.txt)

aux_source_directory(. SOURCE)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.
#
# In this top level CMakeLists.txt, ${CMAKE_PROJECT_NAME} is used to define
# the target library name; in the sub-module's CMakeLists.txt, ${PROJECT_NAME}
# is preferred for the same purpose.
#
# In order to load a library into your app from Java/Kotlin, you must call
# System.loadLibrary() and pass the name of the library defined here;
# for GameActivity/NativeActivity derived applications, the same library name must be
# used in the AndroidManifest.xml file.
add_library(${CMAKE_PROJECT_NAME} SHARED
        # List C/C++ source files with relative paths to this CMakeLists.txt.
        ${X_CORE_SRC}
        ${X_SERVICE_SRC}
        ${X_UTILS_SRC}
        ${SOURCE}
        ${X_SRC_PATH}/ctrl.h
        ${X_SRC_PATH}/ctrl.cpp
        #native-lib.cpp
        )

find_library(
        log-lib
        log)
# Specifies libraries CMake should link to your target library. You
# can link libraries from various origins, such as libraries defined in this
# build script, prebuilt third-party libraries, or Android system libraries.
target_link_libraries(${CMAKE_PROJECT_NAME}
        # List libraries link to the target library
        OpenSLES
        android
        ${log-lib})