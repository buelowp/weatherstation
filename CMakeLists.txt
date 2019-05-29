cmake_minimum_required (VERSION 3.6.0)
project (weatherstation)

set (CMAKE_CXX_STANDARD 14)

set (SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

add_subdirectory (geiger)

add_library (omegafastgpio SHARED IMPORTED)
set_target_properties(omegafastgpio PROPERTIES IMPORTED_LOCATION "${CMAKE_FIND_TARGET_PATH}/usr/lib/libomegafastpio.so")

add_library (onionspi SHARED IMPORTED)
set_target_properties(onionspi PROPERTIES IMPORTED_LOCATION "${CMAKE_FIND_TARGET_PATH}/usr/lib/libonionspi.so")

add_library (onioni2c SHARED IMPORTED)
set_target_properties(onioni2c PROPERTIES IMPORTED_LOCATION "${CMAKE_FIND_TARGET_PATH}/usr/lib/libonioni2c.so")

add_executable (${PROJECT_NAME} ${SOURCE})
target_link_libraries (${PROJECT_NAME} -lm -lomegafastpio -lonionspi -lonioni2c)
