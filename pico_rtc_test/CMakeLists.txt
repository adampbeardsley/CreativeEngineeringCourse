cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(rtc_test C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
pico_sdk_init()

add_executable(rtc_test
  rtc_test.c
)

pico_enable_stdio_usb(rtc_test 1)
pico_enable_stdio_uart(rtc_test 1)

pico_add_extra_outputs(rtc_test)

target_link_libraries(rtc_test pico_stdlib hardware_rtc)
