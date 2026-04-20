add_library(lcdlib lcdlib.c)

target_include_directories(lcdlib
    PUBLIC
        include
)

target_link_libraries(lcdlib
    PRIVATE
        pico_stdlib
    PUBLIC
        hardware_i2c
)