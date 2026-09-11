# 8-Channel Standalone & USB Digital Logic Analyzer

![Altium Designer](https://img.shields.io/badge/EDA-Altium%20Designer-blue.svg)
![Microcontroller](https://img.shields.io/badge/MCU-RP2040%20Dual--Core%20ARM-red.svg)
![Sampling](https://img.shields.io/badge/Sampling-Up%20to%20100MSPS-success.svg)
![Inputs](https://img.shields.io/badge/Logic%20Levels-1.8V%20to%205.5V-orange.svg)
![Display](https://img.shields.io/badge/Display-2.4%22%20IPS%20TFT%20ST7789-purple.svg)
![Firmware](https://img.shields.io/badge/Firmware-Pico%20C%2FC%2B%2B%20SDK-brightgreen.svg)

An open-hardware, 8-channel digital logic analyzer designed around the **Raspberry Pi RP2040** microcontroller. It works both as a **standalone, handheld instrument** with a built-in 2.4 color IPS display, and as a **USB logic analyzer** connected to a PC with software like [Sigrok / PulseView](https://sigrok.org/).

---

## 🌟 Key Features

* **Dual Operation Modes:**
 * **Standalone Mode:** Capture, view, and zoom into waveforms on the built-in 2.4 LCD screen using the 5-way navigation joystick without a computer.
  * **USB PC Mode:** Stream live logic data over USB to PC software (Sigrok / PulseView) for advanced decoding.
* **Universal 1.8V to 5.5V Logic Support:** High-speed 74LVC245A buffer safely level-shifts signals from 1.8V, 2.5V, 3.3V, and 5.0V systems down to the RP2040 3.3V core.
* **Single-Cycle PIO Capture:** Probe channels connect directly to consecutive pins (GPIO0 to GPIO7), allowing the RP2040 Programmable I/O (PIO) state machine to sample all 8 lines simultaneously in 1 clock cycle (in pins, 8).
* **Input Protection:** \,\Omega$ series resistors and low-capacitance PRTR5V0U2X TVS diode clamps protect against static and overvoltage.
* **Switchable ^2C$ Pull-Ups:** 2-position DIP switch enables onboard .2\,\text{k}\Omega$ pull-up resistors on Channel 0 (SDA) and Channel 1 (SCL).

---

## 📐 System Architecture

The instrument is organized into 3 clear operational pipelines: **Signal Ingestion**, **Core Processing**, and **Dual Output (LCD & USB)**.

### 1. End-to-End Signal & Data Pipeline

`mermaid
flowchart TD
    subgraph STAGE1[Stage 1: Signal Ingestion & Protection]
        A[External Probe Inputs (CH0 - CH7)<br/>(1.8V to 5.5V Logic Signals)] --> B[100Ω Series Resistors & TVS Diodes<br/>(Current Limiting & ESD Clamping)]
        B --> C[74LVC245A Level Buffer (@ 3.3V)<br/>(Translates all inputs safely to 3.3V)]
    end

    subgraph STAGE2[Stage 2: High-Speed Capture & Buffering]
        C -->|8 Parallel Lines (GPIO0..7)| D[RP2040 PIO State Machine<br/>(Samples 8 channels in 1 clock cycle)]
        D -->|32-bit Words (4 Samples/Word)| E[Hardware DMA Controller<br/>(Zero-CPU RAM Transfer)]
        E --> F[264 KB Internal SRAM<br/>(Ring Buffer Storage)]
    end

    subgraph STAGE3[Stage 3: Dual-Core Output & Visualization]
        F --> G[Core 0: USB & Protocol Engine<br/>(Handles PC Sigrok/SUMP Commands)]
        F --> H[Core 1: Display & GUI Engine<br/>(Draws waveforms & decodes protocols)]
        
        G -->|USB 2.0 (90Ω Diff Pair)| I[PC (PulseView / Sigrok Software)]
        H -->|62.5MHz SPI1 + DMA| J[2.4-inch Color IPS LCD (ST7789)]
        K[5-Way Navigation Joystick] -->|Zoom / Cursor / Run-Stop| H
    end
`

#### How Data Moves Through the System:
1. **Signal Conditioning:** External probe signals (1.8V–5.5V) pass through current-limiting resistors and ESD TVS diodes into the 74LVC245A buffer, which outputs clean 3.3V logic signals.
2. **Hardware Sampling:** The RP2040 PIO state machine reads all 8 GPIO pins (GPIO0–GPIO7) in a single clock cycle, packing four 8-bit samples into 32-bit words.
3. **Zero-Overhead DMA:** The DMA controller transfers sample words directly from the PIO FIFO into a 64KB ring buffer in SRAM without using CPU cycles.
4. **Dual Output Processing:**
   * **Core 0** monitors the USB port for commands from Sigrok/PulseView and streams sample data over USB CDC.
   * **Core 1** reads the SRAM buffer, renders the 8 waveform traces on the 2.4 color LCD at ~33 FPS, and responds to 5-way joystick inputs.

---

### 2. Power Distribution Architecture

`mermaid
flowchart LR
 USB_IN[USB-C 5V Input<br/>(Charger / Power Bank / PC)] --> FUSE[0.5A PTC Polyfuse<br/>(Overcurrent Protection)]
 FUSE --> LDO[AP2112K-3.3 LDO Regulator<br/>(600mA Low Noise)]
 
 LDO -->|+3.3V Main Rail| BUFFER[74LVC245A Buffer]
 LDO -->|+3.3V Main Rail| LCD[2.4-inch IPS Display]
 LDO -->|+3.3V Main Rail| FLASH[16MB QSPI Flash]
 LDO -->|+3.3V Main Rail| MCU_IO[RP2040 IOVDD / VREG_IN]
 
 MCU_IO --> MCU_INT_LDO[RP2040 Internal LDO]
 MCU_INT_LDO -->|+1.1V Core Rail| MCU_CORE[RP2040 DVDD (Core Logic)]
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

## 💻 Firmware Structure

The firmware source is in the **[/firmware](firmware/)** folder:

`
firmware/
├── CMakeLists.txt # Pico SDK CMake build configuration
├── pico_sdk_import.cmake # Automatic SDK importer
├── README.md # Firmware-specific build guide
└── src/
 ├── main.c # Dual-core bootloader & task scheduler
 ├── logic_analyzer.pio # PIO assembly program (single-cycle 8-bit in)
 ├── capture.c / capture.h # DMA ring buffer & trigger management
 ├── st7789.c / st7789.h # 62.5MHz SPI1 LCD driver with RGB565 rendering
 ├── ui.c / ui.h # 8-trace waveform drawing & 5-way joystick handling
 └── sigrok_protocol.c / .h # SUMP protocol for PulseView/Sigrok over USB CDC
`

---

## 🗂️ Altium Designer Project Structure

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
