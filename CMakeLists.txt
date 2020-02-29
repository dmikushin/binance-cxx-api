cmake_minimum_required(VERSION 3.1.0)

project(binance-cxx-api)

set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 1)
set(PROJECT_VERSION_PATCH 0)

set(CMAKE_CXX_STANDARD 11)

# Search path for CMake include files.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(CURL REQUIRED)

set(JSONCPP_WITH_TESTS OFF CACHE BOOL "")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/jsoncpp EXCLUDE_FROM_ALL)
set_property(TARGET jsoncpp_lib_static PROPERTY POSITION_INDEPENDENT_CODE ON)
set(JSONCPP_INCLUDE_DIRS
	"${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/jsoncpp/include")
set(JSONCPP_LIBRARIES jsoncpp_lib_static)

set(ENABLE_TESTING "" OFF)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/mbedtls EXCLUDE_FROM_ALL)
set(MBEDTLS_LIBRARIES mbedtls)
set(MBEDTLS_INCLUDE_DIRS
	${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/mbedtls/include
	${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/mbedtls/crypto/include)
set_property(TARGET mbedtls PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET mbedx509 PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET mbedcrypto PROPERTY POSITION_INDEPENDENT_CODE ON)

option(LWS_STATIC_PIC "" ON)
option(LWS_WITH_SSL "" OFF)
option(LWS_WITH_MBEDTLS "" ON)
option(LWS_WITH_EXPORT_LWSTARGETS "" OFF)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/libwebsockets EXCLUDE_FROM_ALL)
set(LIBWEBSOCKETS_LIBRARIES websockets)
set(LIBWEBSOCKETS_INCLUDE_DIRS
	"${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/libwebsockets/include"
	"${CMAKE_CURRENT_BINARY_DIR}/ThirdParty/libwebsockets/include")

file(GLOB_RECURSE LIBRARY_SRC "src/*.cpp" "include/*.h")

add_library(${PROJECT_NAME} SHARED ${LIBRARY_SRC})
target_include_directories(${PROJECT_NAME} PRIVATE "${LIBWEBSOCKETS_INCLUDE_DIRS}")
target_include_directories(${PROJECT_NAME} PRIVATE "${JSONCPP_INCLUDE_DIRS}")
target_include_directories(${PROJECT_NAME} PRIVATE "${MBEDTLS_INCLUDE_DIRS}")
target_link_libraries(${PROJECT_NAME} ${LIBWEBSOCKETS_LIBRARIES} ${JSONCPP_LIBRARIES} curl)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(example example.cpp)
target_link_libraries(example ${PROJECT_NAME})

