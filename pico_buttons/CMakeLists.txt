cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(pico_buttons C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(buttons
  buttons.cpp
)

pico_enable_stdio_usb(buttons 1)
pico_enable_stdio_uart(buttons 1)

pico_add_extra_outputs(buttons)

target_link_libraries(buttons pico_stdlib)
