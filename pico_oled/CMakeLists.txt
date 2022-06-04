cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(pico_oled C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(pico_oled
  pico_oled.cpp
)

pico_enable_stdio_usb(pico_oled 1)
pico_enable_stdio_uart(pico_oled 1)

pico_add_extra_outputs(pico_oled)

add_subdirectory(../../pico-ssd1306 ../../pico-ssd1306/build)
target_link_libraries(pico_oled pico_stdlib pico_ssd1306 hardware_i2c)
