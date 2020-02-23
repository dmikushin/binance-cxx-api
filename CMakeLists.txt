cmake_minimum_required(VERSION 3.1.0)

project(binance-cxx-api)

set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 1)
set(PROJECT_VERSION_PATCH 0)

set(CMAKE_CXX_STANDARD 11)

# Search path for CMake include files.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(CURL REQUIRED)
find_package(JsonCpp REQUIRED)

option(LWS_STATIC_PIC "" ON)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/libwebsockets)

file(GLOB_RECURSE LIBRARY_SRC "src/*.cpp" "include/*.h")

add_library(${PROJECT_NAME} SHARED ${LIBRARY_SRC})
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/libwebsockets/include")
target_link_libraries(${PROJECT_NAME} jsoncpp curl websockets)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(example example.cpp)
target_link_libraries(example ${PROJECT_NAME})

