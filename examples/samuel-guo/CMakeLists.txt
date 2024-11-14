add_executable(samuel-guo main.cpp dataGetor.cpp)
target_include_directories(samuel-guo PRIVATE ${CMAKE_CURRENT_SOOURCE_DIR})
target_link_libraries(samuel-guo ${PROJECT_NAME})
