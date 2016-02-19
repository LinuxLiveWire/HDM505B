cmake_minimum_required(VERSION 3.0)

project(HDM505B)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_PREFIX_PATH "/opt/Qt-5.4/5.4/gcc_64;/opt/qwt-6.1.2")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${HDM505B_SOURCE_DIR}/cmake/Modules/" )

enable_testing()

#find_package(GTest REQUIRED PATHS )
#
# Find the QtWidgets library
find_package(Qt5Widgets)
find_package(Qt5SerialPort)
find_package(Qwt REQUIRED)

include_directories(${QWT_INCLUDE_DIR})

set(SOURCE_FILES main.cpp compass.cpp HDM50.h HDM50.cpp dials.h dials.cpp)
add_executable(HDM505B ${SOURCE_FILES})

# Use the Widgets module from Qt 5.
target_link_libraries(HDM505B Qt5::Widgets Qt5::SerialPort ${QWT_LIBRARY})

add_subdirectory(test)
