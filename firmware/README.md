# RP2040 Logic Analyzer Firmware

This directory contains the complete C/C++ firmware for the 8-Channel Standalone & USB Logic Analyzer.

## Architecture

* **Core 0:** Handles high-speed sampling initialization, DMA ring buffer control, PIO triggers, and USB CDC / SUMP protocol streaming to PulseView/Sigrok.
* **Core 1:** Runs the on-PCB 2.4 ST7789 IPS display GUI at ~33 FPS and handles 5-way navigation joystick interactions (Zoom, Cursor, Pan, Run/Stop).
* **PIO Engine (logic_analyzer.pio):** Captures all 8 channels (GPIO0 to GPIO7) in 1 clock cycle at programmable sample rates up to 100+ MSPS.

## Building the Firmware

### Prerequisites
* [Raspberry Pi Pico SDK](https://github.com/raspberrypi/pico-sdk) (v1.5.0 or later)
* CMake (v3.13+)
* ARM GCC Toolchain (rm-none-eabi-gcc)

### Build Steps
`ash
cd firmware
mkdir build && cd build
cmake -DPICO_SDK_PATH=/path/to/pico-sdk ..
make -j4
`

This generates logic_analyzer.uf2.

## Flashing the Board
1. Hold down the **BOOTSEL** tactile switch (SW2) on the PCB.
2. Connect the board to your PC via USB-C.
3. Release the switch; a mass storage drive named RPI-RP2 will appear.
4. Drag and drop logic_analyzer.uf2 onto the RPI-RP2 drive.
5. The device will automatically reboot and start both the LCD display and USB logic analyzer engine.
