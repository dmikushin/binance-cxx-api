cmake_minimum_required(VERSION 3.1.0)

project(binance-cxx-api)

set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 1)
set(PROJECT_VERSION_PATCH 0)

set(CMAKE_CXX_STANDARD 11)

# Search path for CMake include files.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(JSONCPP_WITH_TESTS OFF CACHE BOOL "")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/jsoncpp EXCLUDE_FROM_ALL)
set_property(TARGET jsoncpp_lib_static PROPERTY POSITION_INDEPENDENT_CODE ON)
set(JSONCPP_INCLUDE_DIRS
	"${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/jsoncpp/include")
set(JSONCPP_LIBRARIES jsoncpp_lib_static)

set(ENABLE_TESTING "" OFF)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/mbedtls EXCLUDE_FROM_ALL)
# During the initial CMake run, the mbed targets do not exist yet. Once the
# program is linked, it will become available.
set(MBEDCRYPTO_LIBRARY $<$<TARGET_EXISTS:mbedcrypto>:$<TARGET_FILE:mbedcrypto>>)
set(MBEDTLS_LIBRARY    $<$<TARGET_EXISTS:mbedtls>:$<TARGET_FILE:mbedtls>>)
set(MBEDX509_LIBRARY   $<$<TARGET_EXISTS:mbedx509>:$<TARGET_FILE:mbedx509>>)
set(MBEDTLS_LIBRARIES ${MBEDCRYPTO_LIBRARY} ${MBEDTLS_LIBRARY} ${MBEDX509_LIBRARY})
set(MBEDTLS_INCLUDE_DIRS
	"${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/mbedtls/include"
	"${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/mbedtls/crypto/include")
set_property(TARGET mbedtls PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET mbedx509 PROPERTY POSITION_INDEPENDENT_CODE ON)
set_property(TARGET mbedcrypto PROPERTY POSITION_INDEPENDENT_CODE ON)

option(BUILD_CURL_EXE "" OFF)
option(BUILD_SHARED_LIBS "" OFF)
option(BUILD_TESTING "" OFF)
option(CMAKE_USE_MBEDTLS "" ON)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/curl EXCLUDE_FROM_ALL)
set_property(TARGET libcurl PROPERTY POSITION_INDEPENDENT_CODE ON)
set(CURL_INCLUDE_DIRS
	"${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/curl/include")
set(CURL_LIBRARIES libcurl)

option(LWS_STATIC_PIC "" ON)
option(LWS_WITH_SSL "" OFF)
option(LWS_WITH_MBEDTLS "" ON)
option(LWS_WITH_EXPORT_LWSTARGETS "" OFF)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/libwebsockets EXCLUDE_FROM_ALL)
set(LIBWEBSOCKETS_LIBRARIES websockets)
set(LIBWEBSOCKETS_INCLUDE_DIRS
	"${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty/libwebsockets/include"
	"${CMAKE_CURRENT_BINARY_DIR}/ThirdParty/libwebsockets")

file(GLOB_RECURSE LIBRARY_SRC "src/*.cpp" "include/*.h")

add_library(${PROJECT_NAME} SHARED ${LIBRARY_SRC})
target_include_directories(${PROJECT_NAME} PRIVATE "${LIBWEBSOCKETS_INCLUDE_DIRS}")
target_include_directories(${PROJECT_NAME} PRIVATE "${JSONCPP_INCLUDE_DIRS}")
target_include_directories(${PROJECT_NAME} PRIVATE "${MBEDTLS_INCLUDE_DIRS}")
target_link_libraries(${PROJECT_NAME} ${LIBWEBSOCKETS_LIBRARIES} ${JSONCPP_LIBRARIES} ${CURL_LIBRARIES})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(example example.cpp)
target_link_libraries(example ${PROJECT_NAME})

