add_executable(httpPerformanceTest httpPerformanceTest.cpp)
target_include_directories(httpPerformanceTest PRIVATE "${JSONCPP_INCLUDE_DIRS}")
target_link_libraries(httpPerformanceTest ${PROJECT_NAME})

