cmake_minimum_required (VERSION 3.0.0)
project (weatherstation)

set (CMAKE_CXX_STANDARD 14)
SET (CMAKE_INCLUDE_CURRENT_DIR ON)

file (GLOB SOURCE "*.cpp")

add_subdirectory (geiger)

add_library (omegafastgpio SHARED IMPORTED)
set_target_properties(omegafastgpio PROPERTIES IMPORTED_LOCATION "${CMAKE_FIND_TARGET_PATH}/usr/lib/libomegafastgpio.so")

add_library (onionspi SHARED IMPORTED)
set_target_properties(onionspi PROPERTIES IMPORTED_LOCATION "${CMAKE_FIND_TARGET_PATH}/usr/lib/libonionspi.so")

add_library (onioni2c SHARED IMPORTED)
set_target_properties(onioni2c PROPERTIES IMPORTED_LOCATION "${CMAKE_FIND_TARGET_PATH}/usr/lib/libonioni2c.so")

add_executable (${PROJECT_NAME} ${SOURCE})
target_link_libraries (${PROJECT_NAME} -lm -lomegafastgpio -lonionspi -lonioni2c -lmosquittopp)
