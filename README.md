# C++ Onion.io weather reporting station

Provides an Onion.io interface for sensors that are loosely collected into
weather data. This will be improved with a set of outdoor safe sensors along
the way.

## Requirements

This code requires the following Onion packages to be available

* onioni2c
* onionspi

It also makes use of Kit Bishops newfastgpio library found at
https://github.com/KitBishop/new-gpio. I won't spend time telling you how to
build that here, it's assumed it's available for cross compile and on the
target. You may need to change the library name in CMakeLists.txt.

You also need my Onion cmake toolchain file which is found below. Simply copy
the contents into a file called Toolchain-omega2-mipsel.cmake which exists
somewhere you can easily reference it. Next, modify the line STAGING_DIR to match
you onion installation.

```
# this one is important
SET (CMAKE_SYSTEM_NAME Linux)

set (STAGING_DIR "/path/to/onion/source/staging_dir")
set (TOOLCHAIN_DIR "${STAGING_DIR}/toolchain-mipsel_24kc_gcc-5.4.0_musl-1.1.16")
set (TARGET_DIR "${STAGING_DIR}/target-mipsel_24kc_musl-1.1.16")

# specify the cross compiler
SET (CMAKE_C_COMPILER "${TOOLCHAIN_DIR}/bin/mipsel-openwrt-linux-gcc")
SET (CMAKE_CXX_COMPILER "${TOOLCHAIN_DIR}/bin/mipsel-openwrt-linux-g++")

SET (CMAKE_FIND_TARGET_PATH "${TARGET_DIR}")
SET (CMAKE_SYSROOT ${CMAKE_FIND_TARGET_PATH})

# where is the target environment 
SET (CMAKE_FIND_ROOT_PATH "${TOOLCHAIN_DIR}")

# search for programs in the build host directories
SET (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY) 

set (CMAKE_C_FLAGS "-Os -pipe -mno-branch-likely -mips32r2 -mtune=24kc -fno-caller-saves -fno-plt -fhonour-copts -Wno-error=unused-but-set-variable -Wno-error=unused-result -msoft-float -mips16 -minterlink-mips16 -Wformat -Werror=format-security -fstack-protector -D_FORTIFY_SOURCE=1 -Wl,-z,now -Wl,-z,relro")
set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -ggdb")
set (CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS}")
set (CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS}")
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS} -ggdb")
set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS}")

ADD_DEFINITIONS ("-isystem ${TARGET_DIR}/usr/include")

message(STATUS "Using toolchain file: ${CMAKE_TOOLCHAIN_FILE}.")

# From:
# http://www.vtk.org/Wiki/CMake_Cross_Compiling

# Use the commands:
# mkdir build
# cd build
# cmake -DCMAKE_TOOLCHAIN_FILE=Toolchain-omega2-mipsel.cmake ..
# make
```

## Building

You must have cmake 3.5.0 or newer to build this.

```
cd weatherstation
mkdir build
cd build
cmake -D/path/to/CMAKE_TOOLCHAIN_FILE=Toolchain-omega2-mipsel.cmake ..
make
```

## Installation

Copy it to your onion as you see fit. This does not generate an opkg right now
so you will need to install it on your own.

