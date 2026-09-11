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

## 📌 Hardware Pinout & Peripheral Mapping

| Subsystem | Signal Name | RP2040 Pin | Buffer / Module Pin | Firmware Mapping | Description |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Logic Probes** | RP_LOGIC_0 | **GPIO0** (Pin 2) | 74LVC245A B0 (Pin 18) | LOGIC_PIN_BASE + 0 | Channel 0 Input (I2C SDA / UART RX) |
| **Logic Probes** | RP_LOGIC_1 | **GPIO1** (Pin 3) | 74LVC245A B1 (Pin 17) | LOGIC_PIN_BASE + 1 | Channel 1 Input (I2C SCL / UART TX) |
| **Logic Probes** | RP_LOGIC_2 | **GPIO2** (Pin 4) | 74LVC245A B2 (Pin 16) | LOGIC_PIN_BASE + 2 | Channel 2 Input (SPI SCK) |
| **Logic Probes** | RP_LOGIC_3 | **GPIO3** (Pin 5) | 74LVC245A B3 (Pin 15) | LOGIC_PIN_BASE + 3 | Channel 3 Input (SPI MOSI) |
| **Logic Probes** | RP_LOGIC_4 | **GPIO4** (Pin 6) | 74LVC245A B4 (Pin 14) | LOGIC_PIN_BASE + 4 | Channel 4 Input (SPI MISO) |
| **Logic Probes** | RP_LOGIC_5 | **GPIO5** (Pin 7) | 74LVC245A B5 (Pin 13) | LOGIC_PIN_BASE + 5 | Channel 5 Input (SPI /CS) |
| **Logic Probes** | RP_LOGIC_6 | **GPIO6** (Pin 8) | 74LVC245A B6 (Pin 12) | LOGIC_PIN_BASE + 6 | Channel 6 Input (Trigger / General) |
| **Logic Probes** | RP_LOGIC_7 | **GPIO7** (Pin 9) | 74LVC245A B7 (Pin 11) | LOGIC_PIN_BASE + 7 | Channel 7 Input (Clock / General) |
| **Display SPI** | LCD_SCK | **GPIO10** (Pin 13) | ST7789 SCL (Pin 3) | PIN_LCD_SCK (SPI1) | High-Speed SPI1 Clock (62.5MHz) |
| **Display SPI** | LCD_MOSI | **GPIO11** (Pin 14) | ST7789 SDA (Pin 4) | PIN_LCD_MOSI (SPI1)| SPI1 Master Out Data |
| **Display Control**| LCD_CS | **GPIO13** (Pin 16) | ST7789 CS (Pin 7) | PIN_LCD_CS | Active-Low Chip Select |
| **Display Control**| LCD_DC | **GPIO14** (Pin 17) | ST7789 DC (Pin 6) | PIN_LCD_DC | Data / Command Selection |
| **Display Control**| LCD_RST | **GPIO15** (Pin 18) | ST7789 RES (Pin 5) | PIN_LCD_RST | Hardware Display Reset |
| **Display Control**| LCD_BL | **GPIO27** (Pin 31) | 2N7002 Gate | PIN_LCD_BL | PWM Backlight Dimming |
| **UI Navigation** | NAV_UP | **GPIO16** (Pin 27) | Joystick UP | PIN_NAV_UP | Channel Select / Cursor Up |
| **UI Navigation** | NAV_DOWN | **GPIO17** (Pin 28) | Joystick DOWN | PIN_NAV_DOWN | Channel Select / Cursor Down |
| **UI Navigation** | NAV_LEFT | **GPIO18** (Pin 29) | Joystick LEFT | PIN_NAV_LEFT | Timebase Zoom Out (\,\mu\text{s} \rightarrow 100\,\text{ms}$) |
| **UI Navigation** | NAV_RIGHT | **GPIO19** (Pin 30) | Joystick RIGHT | PIN_NAV_RIGHT | Timebase Zoom In (\,\text{ms} \rightarrow 1\,\mu\text{s}$) |
| **UI Navigation** | NAV_ENTER | **GPIO28** (Pin 32) | Joystick CENTER | PIN_NAV_ENTER | RUN / STOP / Single-Shot Capture |
| **USB 2.0** | USB_DP / DM | **Pins 47 / 46** | USBLC6-2SC6 / J1 | TinyUSB Driver | Full-Speed USB Data Pair (90Ω Diff) |
| **QSPI Flash** | QSPI_SD0..3 | **Pins 51..56** | W25Q128JV | Bootrom / Flash XIP | 16MB High-Speed Flash Bus |

---

## 💻 Firmware Architecture & Directory Layout

The firmware source files are structured in the **[/firmware](firmware/)** folder:

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

### Dual-Core Processing Architecture
* **Core 0 (Capture & USB Engine):**
  * Manages the PIO state machine and DMA buffer transfers.
  * Processes USB CDC communication and SUMP commands from PC logic software.
  * Re-arms continuous capture cycles in standalone mode.
* **Core 1 (Graphics & UI Loop):**
  * Drives the 2.4 \times240$ ST7789 IPS display over hardware SPI1.
 * Polls the 5-way navigation joystick with debouncing.
 * Renders 8 digital waveform traces, timebase grid, cursors, and protocol decode banner at ~33 FPS.

---

## 🗂️ Altium Designer Project Structure

This project uses a **Hierarchical Schematic Architecture** to enforce modular design:

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
1. Hold down the **BOOTSEL** tactile switch (SW2) on the PCB while connecting to a PC via USB-C.
2. Drag and drop the generated **logic_analyzer.uf2** file onto the RPI-RP2 drive.

### Standalone Handheld Operation:
1. Connect any 5V USB-C power source (power bank or charger).
2. Connect the probe GND pin to the target system ground.
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
