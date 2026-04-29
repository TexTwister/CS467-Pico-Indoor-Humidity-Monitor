add_subdirectory(lcdlib)

add_executable(humidity-monitor led_logic.c
)

pico_set_program_name(humidity-monitor "humidity-monitor")
pico_set_program_version(humidity-monitor "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(humidity-monitor 0)
pico_enable_stdio_usb(humidity-monitor 0)

target_link_libraries(humidity-monitor
    PRIVATE
        pico_stdlib
        lcdlib
)

pico_add_extra_outputs(humidity-monitor)