cmake_minimum_required(VERSION 3.7.2)

set (CMAKE_CXX_STANDARD 11)

project(bitrader)
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 1)
set(PROJECT_VERSION_PATCH 0)

# Manual build type selection (for debugging purposes)
#set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_BUILD_TYPE Release)

# Search path for CMake include files.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(CURL REQUIRED)
find_package(LibWebSockets REQUIRED)
find_package(JsonCpp REQUIRED)

file(GLOB_RECURSE LIBRARY_SRC "src/*.cpp" "include/*.h")

add_library(binance SHARED ${LIBRARY_SRC})
target_link_libraries(binance jsoncpp curl websockets)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(example example.cpp)
target_link_libraries(example binance)

