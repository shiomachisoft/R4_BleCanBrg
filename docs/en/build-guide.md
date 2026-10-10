# Firmware Build and Flashing Guide

This document describes the procedures for building and flashing the two firmware programs included in this repository.

1. **`R4_BleCanBrg`**: Main device (for Arduino UNO R4 WiFi / BLE-to-CAN bridge)
2. **`R4_CanEcho`**: Target peer device (for Arduino UNO R4 Minima / CAN echo test)

---

## Prerequisites

1. **Arduino IDE 2.3.8**
   - Download and install Arduino IDE 2.3.8 from the official website.

2. **Board Package Installation**
   - In the Arduino IDE, go to **Tools** -> **Board** -> **Boards Manager...**.
   - Search for **`Arduino Renesas UNO R4 Boards`** (version 1.5.3) and install it.

3. **Library Installation (`R4_BleCanBrg`)**
   - In the Arduino IDE, go to **Tools** -> **Manage Libraries...**.
   - Search for **`ArduinoBLE`** (version 2.1.0) and install it.

---

## 1. Building and Flashing R4_BleCanBrg (Arduino UNO R4 WiFi)

### 1.1 Open Sketch
- Open `src_fw/R4_BleCanBrg/R4_BleCanBrg.ino` from this repository in the Arduino IDE.

### 1.2 Select Board and Port
1. Connect the Arduino UNO R4 WiFi to your PC via a USB-C cable.
2. Go to **Tools** -> **Board** -> **Arduino Renesas UNO R4 Boards** and select **"Arduino UNO R4 WiFi"**.
3. Go to **Tools** -> **Port** and select the recognized COM port for the Arduino UNO R4 WiFi.

### 1.3 Build and Flash
1. Run **Sketch** -> **Verify/Compile** and verify that compilation completes with no errors.
2. Click **Sketch** -> **Upload** to flash the firmware.
3. Once flashing completes, the microcontroller will reboot automatically, and the center bar on the 12×8 LED Matrix will start blinking (advertising / BLE disconnected).

---

## 2. Building and Flashing R4_CanEcho (Arduino UNO R4 Minima)

### 2.1 Open Sketch
- Open `src_fw/R4_CanEcho/R4_CanEcho.ino` from this repository in the Arduino IDE.

### 2.2 Select Board and Port
1. Connect the Arduino UNO R4 Minima to your PC via a USB-C cable.
2. Go to **Tools** -> **Board** -> **Arduino Renesas UNO R4 Boards** and select **"Arduino UNO R4 Minima"**.
3. Go to **Tools** -> **Port** and select the recognized COM port for the Arduino UNO R4 Minima.

### 2.3 Build and Flash
1. Run **Sketch** -> **Verify/Compile** and verify that compilation completes with no errors.
2. Click **Sketch** -> **Upload** to flash the firmware.
3. Once flashing completes, the microcontroller will reboot automatically, and the built-in LED will start blinking at 500 ms intervals.
