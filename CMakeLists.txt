cmake_minimum_required (VERSION 3.6.0)
project (weatherstation)

set (CMAKE_CXX_STANDARD 11)

set (SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

add_executable (${PROJECT_NAME} ${SOURCE})
target_link_libraries (${PROJECT_NAME} -lm -lwiringPi)