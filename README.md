# 8-Channel Standalone & USB Digital Logic Analyzer

![Altium Designer](https://img.shields.io/badge/EDA-Altium%20Designer-blue.svg)
![Microcontroller](https://img.shields.io/badge/MCU-RP2040%20Dual--Core%20ARM-red.svg)
![Sampling](https://img.shields.io/badge/Sampling-Up%20to%20100MSPS-success.svg)
![Inputs](https://img.shields.io/badge/Logic%20Levels-1.8V%20to%205.5V-orange.svg)
![Display](https://img.shields.io/badge/Display-2.4%22%20IPS%20TFT%20ST7789-purple.svg)
![Firmware](https://img.shields.io/badge/Firmware-Pico%20C%2FC%2B%2B%20SDK-brightgreen.svg)

A high-speed, 8-channel digital logic analyzer designed around the **Raspberry Pi RP2040** microcontroller. This instrument operates both as a **standalone, handheld on-PCB visualizer** with an integrated 2.4 color IPS LCD and 5-way joystick navigation, or as a **high-throughput USB logic analyzer** compatible with open-source logic analysis suites like [Sigrok / PulseView](https://sigrok.org/).

---

## 🌟 Key Features

* **Dual-Mode Operation:**
 * **Standalone Mode:** View 8-channel waveforms, trigger states, timebase cursors, and real-time protocol decoding directly on the 2.4 IPS screen without needing a PC.
  * **USB Host Mode:** Stream digital waveforms to PC via USB 2.0 Full-Speed using standard Sigrok/PulseView drivers.
* **Wide Voltage Range (1.8V to 5.5V):** Powered by an onboard 74LVC245A high-speed level translator and input buffer, safe for 1.8V, 2.5V, 3.3V, and 5.0V logic systems.
* **Single-Cycle PIO Capture:** Probe channels mapped to consecutive pins (GPIO0 to GPIO7) allowing the RP2040 Programmable I/O (PIO) state machine to capture all 8 lines in 1 clock cycle (in pins, 8).
* **Robust Input Protection:**
  * \,\Omega$ series current-limiting resistors on each channel.
  * Low-capacitance (.0\,\text{pF}$) PRTR5V0U2X TVS clamping diode array.
* **Switchable ^2C$ Bus Pull-Ups:** Onboard 2-position DIP switch connects .2\,\text{k}\Omega$ pull-ups to CH0 (SDA) and CH1 (SCL) for debugging un-terminated buses.
* **Clean Power Subsystem:** 600mA Low-Dropout AP2112K-3.3 regulator with .5\,\text{A}$ PTC polyfuse and USBLC6-2SC6 high-speed USB ESD protection.

---

## 📐 System Architecture

`mermaid
flowchart LR
    subgraph INPUTS[Input Probe Stage]
        PROBES[8x Probe Inputs<br/>(1.8V - 5.5V)]
        DAMPING[100Ω Resistors<br/>& TVS Protection]
        PULLUPS[DIP Switch 2.2kΩ<br/>(I2C SDA / SCL)]
        BUFFER[74LVC245A Buffer<br/>(VCC = 3.3V)]
    end

    subgraph MCU[RP2040 Microcontroller]
        PIO[PIO Engine<br/>(Single-cycle 8-bit In)]
        RAM[264KB Ring Buffer]
        CORE1[Core 1 Graphics<br/>& Protocol Engine]
    end

    subgraph UI[Onboard User Interface]
        LCD[2.4-inch IPS LCD<br/>(ST7789 320x240)]
        JOYSTICK[5-Way Nav Switch<br/>(Zoom, Pan, Run/Stop)]
    end

    subgraph POWER_USB[Power & USB]
        USBC[USB-C Receptacle<br/>(5.1k CC Pull-downs)]
        LDO[AP2112K-3.3 LDO<br/>(600mA Regulator)]
    end

    PROBES --> DAMPING
    DAMPING --> PULLUPS
    PULLUPS --> BUFFER
    BUFFER == RP_LOGIC[0..7]<br/>(GPIO0..GPIO7) ==> PIO
    PIO --> RAM
    RAM --> CORE1
    CORE1 == High-Speed SPI1 + DMA ==> LCD
    JOYSTICK -->|GPIO16..19, 28| CORE1
    USBC --> LDO
    LDO ==>|+3.3V System Rail| MCU
    LDO ==>|+3.3V System Rail| UI
    LDO ==>|+3.3V System Rail| INPUTS
`

---

## 💻 Firmware Architecture & Code Structure

The firmware is located in the **[/firmware](firmware/)** folder and is built on the **Raspberry Pi Pico C/C++ SDK**. It leverages the dual-core architecture and hardware DMA to run real-time capture and 33 FPS UI rendering in parallel without dropping samples.

`
firmware/
├── CMakeLists.txt              # Pico SDK CMake build configuration
├── pico_sdk_import.cmake       # Automatic SDK importer
├── README.md                   # Firmware-specific build guide
└── src/
    ├── main.c                  # Dual-core bootloader & task scheduler
    ├── logic_analyzer.pio      # PIO assembly program (single-cycle 8-bit in)
    ├── capture.c / capture.h   # DMA ring buffer & trigger management
    ├── st7789.c / st7789.h     # 62.5MHz SPI1 LCD driver with RGB565 rendering
    ├── ui.c / ui.h             # 8-trace waveform drawing & 5-way joystick handling
    └── sigrok_protocol.c / .h  # SUMP protocol for PulseView/Sigrok over USB CDC
`

### Dual-Core Task Division
* **Core 0 (Capture & USB Engine):**
  * Manages the PIO state machine and DMA buffer transfers.
  * Listens on USB CDC for SUMP commands from PC software (Sigrok / PulseView).
  * Automatically re-arms continuous capture cycles when running in standalone mode.
* **Core 1 (Graphics & UI Loop):**
  * Exclusively manages the 2.4 \times240$ ST7789 IPS display over SPI1.
 * Polls the 5-way navigation joystick with software debouncing.
 * Renders 8 digital waveform traces, timebase grid, cursors, and protocol decode text at ~33 FPS.

---

## 🔍 Hardware-to-Firmware Pin Verification Matrix

All pins in the schematic design have been 100% matched and verified against the firmware source code:

| Function | Signal Name | Schematic Pin | Firmware Definition | Target File | Verification Status |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Logic Probe 0** | RP_LOGIC_0 | **GPIO0** (Pin 2) | LOGIC_PIN_BASE + 0 | capture.h / logic_analyzer.pio | ✅ Verified (PIO IN Pin 0) |
| **Logic Probe 1** | RP_LOGIC_1 | **GPIO1** (Pin 3) | LOGIC_PIN_BASE + 1 | capture.h / logic_analyzer.pio | ✅ Verified (PIO IN Pin 1) |
| **Logic Probe 2** | RP_LOGIC_2 | **GPIO2** (Pin 4) | LOGIC_PIN_BASE + 2 | capture.h / logic_analyzer.pio | ✅ Verified (PIO IN Pin 2) |
| **Logic Probe 3** | RP_LOGIC_3 | **GPIO3** (Pin 5) | LOGIC_PIN_BASE + 3 | capture.h / logic_analyzer.pio | ✅ Verified (PIO IN Pin 3) |
| **Logic Probe 4** | RP_LOGIC_4 | **GPIO4** (Pin 6) | LOGIC_PIN_BASE + 4 | capture.h / logic_analyzer.pio | ✅ Verified (PIO IN Pin 4) |
| **Logic Probe 5** | RP_LOGIC_5 | **GPIO5** (Pin 7) | LOGIC_PIN_BASE + 5 | capture.h / logic_analyzer.pio | ✅ Verified (PIO IN Pin 5) |
| **Logic Probe 6** | RP_LOGIC_6 | **GPIO6** (Pin 8) | LOGIC_PIN_BASE + 6 | capture.h / logic_analyzer.pio | ✅ Verified (PIO IN Pin 6) |
| **Logic Probe 7** | RP_LOGIC_7 | **GPIO7** (Pin 9) | LOGIC_PIN_BASE + 7 | capture.h / logic_analyzer.pio | ✅ Verified (PIO IN Pin 7) |
| **LCD SPI Clock** | LCD_SCK | **GPIO10** (Pin 13) | PIN_LCD_SCK (10) | st7789.h | ✅ Verified (SPI1 SCK) |
| **LCD SPI MOSI** | LCD_MOSI | **GPIO11** (Pin 14) | PIN_LCD_MOSI (11) | st7789.h | ✅ Verified (SPI1 TX) |
| **LCD Chip Select**| LCD_CS | **GPIO13** (Pin 16) | PIN_LCD_CS (13) | st7789.h | ✅ Verified (GPIO Output) |
| **LCD Data/Command**| LCD_DC | **GPIO14** (Pin 17) | PIN_LCD_DC (14) | st7789.h | ✅ Verified (GPIO Output) |
| **LCD Reset** | LCD_RST | **GPIO15** (Pin 18) | PIN_LCD_RST (15) | st7789.h | ✅ Verified (GPIO Output) |
| **LCD Backlight** | LCD_BL | **GPIO27** (Pin 31) | PIN_LCD_BL (27) | st7789.h | ✅ Verified (PWM Slice 5B) |
| **Joystick UP** | NAV_UP | **GPIO16** (Pin 27) | PIN_NAV_UP (16) | ui.h | ✅ Verified (Internal Pull-up) |
| **Joystick DOWN** | NAV_DOWN | **GPIO17** (Pin 28) | PIN_NAV_DOWN (17) | ui.h | ✅ Verified (Internal Pull-up) |
| **Joystick LEFT** | NAV_LEFT | **GPIO18** (Pin 29) | PIN_NAV_LEFT (18) | ui.h | ✅ Verified (Internal Pull-up) |
| **Joystick RIGHT**| NAV_RIGHT| **GPIO19** (Pin 30) | PIN_NAV_RIGHT (19)| ui.h | ✅ Verified (Internal Pull-up) |
| **Joystick PUSH** | NAV_ENTER| **GPIO28** (Pin 32) | PIN_NAV_ENTER (28)| ui.h | ✅ Verified (Internal Pull-up) |

---

## 🗂️ Altium Designer Project Structure

This project uses a **Hierarchical Schematic Architecture** to enforce clean modular design and strict electrical rule checking:

`
RP2040_Logic_Analyzer.PrjPcb
├── Top.SchDoc # Master root sheet with Sheet Symbols & Ports
├── 01_Power_USB.SchDoc # USB-C receptacle, PTC fuse, TVS clamp, and 3.3V LDO
├── 02_MCU_RP2040.SchDoc # RP2040 MCU, 12MHz crystal, 16MB QSPI Flash, SWD, Reset
├── 03_Input_Buffer.SchDoc # 10-pin probe header, ESD clamps, DIP switch, 74LVC245 buffer
└── 04_Display_UI.SchDoc # 2.4 ST7789 IPS LCD interface and 5-way joystick matrix
`

---

## 📊 Key Component Bill of Materials (BOM)

| Component | Part Number | Package | Description |
| :--- | :--- | :--- | :--- |
| **Microcontroller** | RP2040 | QFN-56 (7x7mm) | Dual-Core ARM Cortex-M0+ MCU |
| **Input Buffer** | 74LVC245APW,118 | TSSOP-20 | 8-Bit Transceiver / Level Shifter |
| **LDO Regulator** | AP2112K-3.3TRG1 | SOT-23-5 | 3.3V 600mA Low-Dropout Linear Regulator |
| **QSPI Flash** | W25Q128JVSIQ | SOIC-8 (208-mil)| 16MB Quad-SPI NOR Flash Memory |
| **USB TVS** | USBLC6-2SC6 | SOT-23-6 | High-Speed USB ESD Protection Diode |
| **Input TVS** | PRTR5V0U2X,215 | SOT-143B | Dual Low-Capacitance TVS Clamp Diodes (x4) |
| **Display** | ST7789-2.4-SPI | 8-Pin Module | 2.4 320x240 IPS Color TFT LCD |
| **Joystick** | SKRHAAE010 | SMT Module | 5-Way Directional Navigation Switch |
| **12MHz XTAL** | ABM8G-12.000MHZ-18 | 3.2x2.5mm SMD | 12.000MHz Master Crystal Oscillator |
| **USB Connector** | TYPE-C-31-M-12 | 16-Pin Hybrid | USB Type-C Receptacle |

---

## 🚀 How to Build, Flash & Use

### Building & Flashing the Firmware:
`ash
# Clone the repository
git clone https://github.com/anaswarramesh/RP2040_Logic_Analyzer.git
cd RP2040_Logic_Analyzer/firmware

# Create build directory and compile
mkdir build && cd build
cmake -DPICO_SDK_PATH=/path/to/pico-sdk ..
make -j4
`
1. Hold down the **BOOTSEL** tactile switch (SW2) on the PCB while connecting it to your PC via USB-C.
2. Drag and drop the generated **logic_analyzer.uf2** file onto the RPI-RP2 drive.

### Standalone Handheld Operation:
1. Connect any 5V USB-C power source (power bank or charger).
2. Connect the probe GND pin to your Device Under Test (DUT) ground.
3. Attach probe channels CH0–CH7 to the target digital lines.
4. If testing an un-terminated ^2C$ bus, turn **DIP Switch 1 & 2** ON to activate the .2\,\text{k}\Omega$ pull-ups.
5. Use the **5-Way Joystick**:
 * **Left / Right:** Adjust horizontal timebase (\,\mu\text{s}/\text{div} \leftrightarrow 100\,\text{ms}/\text{div}$).
 * **Up / Down:** Select channel and move measurement markers.
 * **Center Push:** Toggle between **RUN** (continuous capture) and **STOP** (freeze buffer).

### USB PC Operation (Sigrok / PulseView):
1. Connect to PC via USB-C.
2. Open **PulseView**.
3. Connect to device using driver Openbench Logic Sniffer & SUMP compatibles or Raspberry Pi Pico Logic Analyzer.
4. Set sample rate up to **100 MSPS** and capture signals directly into PulseView protocol decoders.

---

## 📜 License & Acknowledgments
* **Hardware Design:** Open-source hardware under the **CERN Open Hardware Licence Version 2 - Strongly Reciprocal (CERN-OHL-S)**.
* **Firmware:** Open-source software under the **MIT License**.
