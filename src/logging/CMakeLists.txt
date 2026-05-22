add_library(logging logging.c)

target_include_directories(logging
    PUBLIC
        include
)

target_link_libraries(logging
    PUBLIC
        pico_stdlib
)