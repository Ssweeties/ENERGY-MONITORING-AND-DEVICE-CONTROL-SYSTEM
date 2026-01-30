# 🌐 Energy Monitoring & Device Control System (ESP32 + Firebase + App)

## 📌 Project Title
**IoT-Based Energy Monitoring and Device Control System Using ESP32, ACS712, ZMPT101B, OLED, RFID, RELAY**

## 📖 Description
This project is a comprehensive IoT solution designed to monitor electrical parameters (Voltage, Current) and Calculator Energy Power (Power, Energy) for two separate devices using **ACS712** and **ZMPT101B**. It integrates **RFID*** authentication for secure access control, ensuring that only authorized users can activate the devices. Real-time data is synchronized with **Google Firebase** for remote monitoring and control, and key metrics are displayed locally on an **OLED SSD1306 screen** and **App**.

## 🛠️ How to Use

### 🔧 Requirements
*   **ESP-IDF**: [v5.5.2](https://dl.espressif.com/dl/esp-idf/?idf=4.4)
*   **Cloud Platform**: Google Firebase (Realtime Database)
*   **Hardware Components**:
    *   ESP32 DevKit 32D
    *   2x ACS712 Current Sensors (20A)
    *   2x ZMPT101B Voltage Sensors
    *   MFRC522 RFID Reader
    *   OLED SSD1306 Display (I2C)
    *   2x Relays 5V
    *   2x AC Loads (Bulbs, Fans, etc.)
    *   Compact AC/DC power module HLK-10M05

### ☁️ Firebase Configuration
The system uses the following data structure in Firebase Realtime Database:
*   **`dev1/`** & **`dev2/`**: Stores electrical values (`U`, `I`, `P`, `E`) and Relay `Status`.
*   **`User/`**: Tracks cumulative energy usage per user (`E`, `E1`, `E2`).
*   **`login`, `logout`**: System state flags.

### 📁 Project Structure
```text
├── CMakeLists.txt
├── README.md
├── src
│   ├── main.c           # System init & task setup
│   ├── main.h           # Configuration, Pin Definitions, and Structures
│   └── components
│       ├── power.c      # Drivers for ACS712 & ZMPT101B sensors
│       ├── power.h      # Function declarations and data structures for power sensors
│       ├── RFID.c       # MFRC522 Driver
│       ├── RFID.h       # RFID interface & register definitions
│       ├── firebase.c   # HTTPS REST Client for Firebase RTDB
│       ├── firebase.h   # Function declarations for Firebase
│       ├── OLED.c       # I2C Driver for SSD1306 Display
│       ├── OLED.h       # SSD1306 Commands, Font Data & Function Declarations for OLED
│       ├── login.c      # Login handle
│       ├── login.h      # Function declarations for Login
│       ├── logout.c     # Logout handle
│       └── logout.h     # Function declarations for Logout
```

## 🧩 Hardware Connections

| Component | Pin Name | ESP32 GPIO | Notes |
| :--- | :--- | :--- | :--- |
| **OLED Display** | SDA | **GPIO 21** | I2C Data |
| | SCL | **GPIO 22** | I2C Clock |
| **RFID (RC522)** | SDA (SS) | **GPIO 5** | SPI Chip Select |
| | SCK | **GPIO 18** | SPI Clock |
| | MISO | **GPIO 19** | SPI MISO |
| | MOSI | **GPIO 23** | SPI MOSI |
| | RST | **GPIO 4** | Reset |
| **Sensors** | ACS712 (Dev 1) | **GPIO 34** | Analog Input (ADC1_6) |
| | ZMPT101B (Dev 1)| **GPIO 35** | Analog Input (ADC1_7) |
| | ACS712 (Dev 2) | **GPIO 32** | Analog Input (ADC1_4) |
| | ZMPT101B (Dev 2)| **GPIO 33** | Analog Input (ADC1_5) |
| **Controls** | Relay 1 | **GPIO 14** | Active High |
| | Relay 2 | **GPIO 27** | Active High |

## 🌐 System Behavior & Workflow
1.  **Startup**: Connects to Wi-Fi (Credentials in `main.h`) and syncs with Firebase.
2.  **Idle State**: OLED displays "Please Login" or Wi-Fi status. Relays are OFF.
3.  **Authentication**:
    *   User scans RFID card.
    *   System validates UID against hardcoded list (`EE:E0:E8:00`, etc.).
    *   If valid: Logs in, enables Relays, starts metering.
4.  **Active State**:
    *   Reads Voltage/Current every **1.5s**.
    *   Calculates Power (W) and accumulated Energy (Wh).
    *   Updates OLED with real-time values.
    *   Syncs data to Firebase.
5.  **Remote Control**:
    *   Change `dev1/Status` to `true`/`false` in Firebase to toggle relay remotely.
    *   Set `logout` to `true` in Firebase to force end the session.

## 📈 Functional Diagram
<p align="center">
  <img src="https://github.com/Ssweeties/ENERGY-MONITORING-AND-DEVICE-CONTROL-SYSTEM/blob/367211c614a768c98c3d7ebf71aa34f40d9ed9d1/Diagram.jpg?raw=true" alt="Diagram">
</p>
## 📺 Example Output

### Terminal Log
```text
I (5240) WIFI: Connected, IP: 192.168.1.105
I (5500) SYSTEM: Ready
I (12000) RFID: Card Detected: EE:E0:E8:00
I (12100) LOGIN: Access Granted
I (13500) PWR: Dev1: 220V 0.5A 110W | Dev2: 0V 0A 0W
I (13600) FIREBASE: Data Pushed
```

### Dashboard (Firebase)
*   **Real-time**: View Voltage/Current graphs.
*   **Control**: Toggle switches for Device 1 & 2.

### System Protection
*   **Noise Filtering**: Software filters for ACS712 noise (<50mA) and ZMPT101B (<10V).
*   **Data Safety**: Uses Mutex for thread-safe Firebase operations.
*   **WiFi Reconnect**: Aautomatically attempts to reconnect if signal is lost.

## 🔌 I/O Summary

| Signal | Type | Description |
| :--- | :--- | :--- |
| `dev1.power` | Output | Calculated Power (W) sent to Cloud/OLED |
| `dev1.energy` | Output | Accumulated Energy (Wh) per session |
| `relay1_on` | Input/Output | Controlled via RFID logic OR Firebase |
| `is_logged_in` | Flag | Global system security state |

## ✅ Result Summary
*   ✅ **Accurate Metering**: Calibrated for standard AC loads.
*   ✅ **Secure**: RFID-based physical access control.
*   ✅ **Responsive**: Low-latency control via Firebase Stream.
*   ✅ **User-Friendly**: Clear OLED interface for local feedback.

## 👤 Author
**Trần Huỳnh**  
Major: Computer Engineering Technology  
Faculty of Electrical-Electronics, HCMUTE  
Email: huynhtran30112004@gmail.com  
