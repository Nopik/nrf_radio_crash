project(crasher)

cmake_minimum_required(VERSION 3.8)
set(CMAKE_CXX_STANDARD 11)

include_directories(
        src
        src/common
        src/platform/nrf52840
        src/platform/nrf52840/drivers
        build/include
        third_party/
)

set(INCLUDES
        src/platform/nrf52840/FreeRTOSConfig.h
        src/wrappers/wmutex.h
        src/wrappers/wtask.h
)

set(TB_INCLUDES
        src/crasher/error_types.h
        src/crasher/sdk_config.h
)

set(SOURCES
        src/crasher/hardware/nrf_radio.cpp
        src/crasher/application_crasher.cpp
        src/wrappers/wtask.cpp
        src/wrappers/wmutex.cpp
        src/crasher/main.cpp
        )

add_executable(crasher ${TB_INCLUDES} ${SOURCES})
