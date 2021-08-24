add_executable(multithreading multithreading.cpp)
target_include_directories(multithreading PRIVATE "${JSONCPP_INCLUDE_DIRS}")
target_link_libraries(multithreading ${PROJECT_NAME})
