# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
cmake_minimum_required(VERSION 3.13)

if(NOT PICO_BOARD)
    set(PICO_BOARD pico_w)
endif()

if(NOT PICO_COMPILER)
    set(PICO_COMPILER pico_arm_cortex_m0plus_gcc)
endif()

include(pico_sdk_import.cmake)

project(pico_indoor_humidity_monitor C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

pico_sdk_init()

# I included the dht20 code below, while Loren's is split up in /src/CMakeLists.txt and /src/lcdlib/CMakeLists.txt
add_executable(pico_indoor_humidity_monitor
    src/main.c
    src/dht20lib/dht20.c
)

target_include_directories(pico_indoor_humidity_monitor PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/dht20lib/include
)

target_link_libraries(pico_indoor_humidity_monitor
    pico_stdlib
    hardware_i2c
)

pico_enable_stdio_usb(pico_indoor_humidity_monitor 1)
pico_enable_stdio_uart(pico_indoor_humidity_monitor 0)

pico_add_extra_outputs(pico_indoor_humidity_monitor)
