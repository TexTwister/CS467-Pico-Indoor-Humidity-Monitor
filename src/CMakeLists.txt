add_subdirectory(lcdlib)
add_subdirectory(ledlib)

add_library(dht20lib dht20lib/dht20.c)

target_include_directories(dht20lib
    PUBLIC
        dht20lib/include
)

target_link_libraries(dht20lib
    PRIVATE
        pico_stdlib
    PUBLIC
        hardware_i2c
)

if($ENV{CHECK_CRC} MATCHES 0)
    target_compile_options(dht20lib
        PRIVATE
            -DCHECK_CRC=0
    )
endif()

add_executable(humidity-monitor main.c)

pico_set_program_name(humidity-monitor "humidity-monitor")
pico_set_program_version(humidity-monitor "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(humidity-monitor 0)
pico_enable_stdio_usb(humidity-monitor 1)

target_link_libraries(humidity-monitor
    PRIVATE
        pico_stdlib
        dht20lib
        lcdlib
        ledlib
)

pico_add_extra_outputs(humidity-monitor)
