cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(sun_alarm)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

add_executable(sun_alarm)

pico_generate_pio_header(sun_alarm ${CMAKE_CURRENT_LIST_DIR}/pico_ws2811.pio)

target_sources(sun_alarm PRIVATE sun_alarm.cpp)

pico_enable_stdio_usb(sun_alarm 1)
pico_enable_stdio_uart(sun_alarm 1)

add_subdirectory(../../pico-ssd1306 ../../pico-ssd1306/build)
target_link_libraries(sun_alarm PRIVATE
  pico_stdlib
  hardware_pio
  hardware_dma
  hardware_irq
  hardware_rtc
  pico_ssd1306
  hardware_i2c)

pico_add_extra_outputs(sun_alarm)
