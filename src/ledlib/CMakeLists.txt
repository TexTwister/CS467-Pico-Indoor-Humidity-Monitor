add_library(ledlib 
    led_logic.c
    led_validation_logic.c
)

target_include_directories(ledlib
    PUBLIC
        include
)

target_link_libraries(ledlib
    PRIVATE
        pico_stdlib
)