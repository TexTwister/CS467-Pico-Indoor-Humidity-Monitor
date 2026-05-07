add_subdirectory(lcdlib)

<<<<<<< HEAD
<<<<<<< Updated upstream
<<<<<<< HEAD
add_executable(humidity-monitor led_logic.c
=======
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
add_executable(humidity-monitor main.c led_logic.c
>>>>>>> Stashed changes
)
=======
=======
>>>>>>> 59fc737af58ac847b8d550ff25f45a9d95e3687a
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
<<<<<<< HEAD

if($ENV{CHECK_CRC} MATCHES 0)
    target_compile_options(dht20lib
        PRIVATE
            -DCHECK_CRC=0
    )
endif()

add_executable(humidity-monitor main.c)
>>>>>>> main
=======
add_executable(humidity-monitor main.c led_logic.c
)
>>>>>>> 59fc737af58ac847b8d550ff25f45a9d95e3687a

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
)

pico_add_extra_outputs(humidity-monitor)
