# LoRaWAN PawPal SmartFeeder

## Overview
The **PawPal SmartFeeder** is a modular and energy-efficient IoT prototype designed to automate pet feeding using real-time monitoring and remote management. It leverages **LoRaWAN communication** and **Azure IoT Hub** to ensure reliable, long-range connectivity and seamless user interaction. The system integrates two Arduino boards to separate hardware management and communication logic.

## Project Focus: Communication and Code Architecture

### Communication System
At the heart of the PawPal system lies a **dual-microcontroller setup**:
- **Arduino Uno**: Handles all physical interactions, including sensors and actuators.
- **Arduino Pro Mini**: Manages communication between the Uno and LoRaWAN network.

The two boards exchange data using **SoftwareSerial**, ensuring efficient separation between hardware control and wireless communication.

#### LoRaWAN Integration
- The **RN2483 LoRa module** is used for long-range, low-power wireless communication.
- The device supports **uplink (sensor data)** and **downlink (user commands)**, handled intelligently by toggling between active serial lines.
- Messages are processed via **The Things Network (TTN)** and visualized through **Azure IoT Hub**, enabling remote feeding control and data tracking.

The communication protocol includes:
- **Start/Stop signaling** for data consistency.
- **Custom message formats** for interpreting commands and sensor data.
- **Scheduling strategies** to respect LoRaWAN duty-cycle limits.

### Code Highlights

#### `ArdUno_sensors.ino`
- Handles sensor readings:
  - **Ultrasonic sensor** (food level)
  - **LDR** (bowl content detection)
  - **Water level sensor**
- Controls:
  - **Stepper motor** (food dispensing)
  - **Servo motor** (water valve)
- Executes routines based on **downlink payloads (0–3)**:
  - 0 → Read sensors
  - 1 → Refill water
  - 2 → Refill food
  - 3 → Refill both
- Sends **uplink messages** to Pro Mini and enters **sleep mode** between actions.

#### `ArdMini.ino`
- Manages two serial connections:
  - With Arduino Uno
  - With LoRaWAN module (RN2483)
- Sends and receives payloads:
  - Uplink: Sensor data → TTN
  - Downlink: User commands → Uno
- Includes **demo mode** with fake downlink commands for testing.
- Contains fallback and retry logic for network errors.

## Repository Contents
- `ArdUno_sensors.ino` — Arduino Uno code (sensors & actuators)
- `ArdMini.ino` — Arduino Pro Mini code (communication)
- `ReadMe.pdf` — Hardware and software summary
- `FinalReport_Group15.pdf` — Full academic report
