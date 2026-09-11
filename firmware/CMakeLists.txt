cmake_minimum_required(VERSION 3.13)

# Pull in SDK (must be in environment or submodule)
include(pico_sdk_import.cmake)

project(RP2040_Logic_Analyzer C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(logic_analyzer
    src/main.c
    src/capture.c
    src/st7789.c
    src/ui.c
    src/sigrok_protocol.c
)

# Generate PIO header file from logic_analyzer.pio
pico_generate_pio_header(logic_analyzer /src/logic_analyzer.pio)

# Link hardware libraries
target_link_libraries(logic_analyzer
    pico_stdlib
    pico_multicore
    hardware_pio
    hardware_dma
    hardware_spi
    hardware_pwm
    hardware_vreg
    hardware_clocks
)

# Enable USB CDC stdio output, disable UART stdio
pico_enable_stdio_usb(logic_analyzer 1)
pico_enable_stdio_uart(logic_analyzer 0)

# Create UF2 file for dragging and dropping onto RP2040 BOOTSEL drive
pico_add_extra_outputs(logic_analyzer)
