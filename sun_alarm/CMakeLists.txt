cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(sun_alarm)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

add_executable(sun_alarm)

pico_generate_pio_header(sun_alarm ${CMAKE_CURRENT_LIST_DIR}/pico_ws2811.pio)

target_sources(sun_alarm PRIVATE sun_alarm.cpp)

target_link_libraries(sun_alarm PRIVATE pico_stdlib hardware_pio hardware_dma hardware_irq)

pico_add_extra_outputs(sun_alarm)
