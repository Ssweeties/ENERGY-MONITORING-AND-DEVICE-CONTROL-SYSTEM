# Energy Monitoring & Device Control System (ESP32 + Firebase + App)

##Project Title
**IoT-Based Energy Monitoring and Device Control System Using ESP32, ACS712, ZMPT101B, OLED, RFID, RELAY**

## Description
This project is a comprehensive IoT solution designed to monitor electrical parameters (Voltage, Current) and Calculator Energy Power (Power, Energy) for two separate devices using **ACS712** and **ZMPT101B**. It integrates **RFID*** authentication for secure access control, ensuring that only authorized users can activate the devices. Real-time data is synchronized with **Google Firebase** for remote monitoring and control, and key metrics are displayed locally on an **OLED SSD1306 screen** and **App**.

## How to Use
### Before flashing, you must configure your network and Firebase credentials.
1.  Open the project in VS Code.
2.  Navigate to `src/main.h`.
3.  Update the following macros with your credentials:
    ```c
    #define WIFI_SSID           "Your_WiFi_Name"      // Replace with your WiFi SSID
    #define WIFI_PASSWORD       "Your_WiFi_Password"  // Replace with your WiFi Password
    #define FIREBASE_API_KEY    "Your_Firebase_Web_API_Key"
    #define FIREBASE_DB_URL     "https://your-project-id.firebaseio.com/"
    ```
4.  (Optional) Update `VALID_UIDS` in `src/main.c` to add valid UID.
### Build and Flash
1.  **Open Terminal**: Use the ESP-IDF terminal in VS Code.
2.  **Set Target** (First time only):
    ```bash
    idf.py set-target esp32
    ```
3.  **Build the Project**:
    ```bash
    idf.py build
    ```
4.  **Flash and Monitor**:
    *   Connect your ESP32 via USB.
    *   Find the COM port (e.g., `COM3` on Windows, `/dev/ttyUSB0` on Linux).
    *   Run the command:
        ```bash
        idf.py -p COM3 flash monitor
        ```
## Requirements
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

## Firebase Configuration
The system uses the following data structure in Firebase Realtime Database:
*   **`dev1/`** & **`dev2/`**: Stores electrical values (`U`, `I`, `P`, `E`) and Relay `Status`.
*   **`User/`**: Tracks cumulative energy usage per user (`E`, `E1`, `E2`).
*   **`login`, `logout`**: System state flags.

## Project Structure
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

## Hardware Connections

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
- The ADC samples the ACS712 and ZMPT101B AC signals biased at Vin/2.
- ADC operates at 3.3V, so the ACS712 and ZMPT101B **Vcc** must be scaled down to 3.3V.
  
## System Behavior & Workflow
1. **Startup**: The OLED displays "CONNECT WIFI..." to require the user to connect to Wi-Fi. Once Wi-Fi is connected, the sensors and Firebase will be initialized.
2. **Login**:
   - The OLED displays "PLEASE LOGIN", requiring the user to initiate the login process via the app to set the login flag.
   - Once the login flag is set, the RFID task will be created and will wait for the user to scan an RFID card.
   - If the RFID is valid, the login process will be completed, the login flag will be cleared, and the system will be set up before entering the main task.
3. **Setup**: After a valid RFID card is scanned, the relay will be activated and turned on. The system then retrieves the last session data from Firebase to calculate the next energy value. After that, the system enters the main task.
4. **Main task**:
   - Read voltage and current values from the sensors and calculate power and energy.
   - Display the measured values on the OLED.
   - Update the values to Firebase.
   - Store energy data to Firebase every 5 seconds so it can be retrieved in the next session when the user logs in again.
5. **Remote Control**:
   - Change `dev1/Status` to `true` or `false` in Firebase to remotely toggle the relay.
   - If the relay is turned off, the system will not measure power.
6. **Logout**:
   - If the user wants to log out, the user can set the logout flag via the app.
   - Once the logout flag is set, the system will stop measuring power.
   - The relay will be turned off and deactivated.
   - The last energy data will be stored on Firebase so it can be retrieved in the next session when the user logs in again.
## 📈 Functional Diagram
<p align="center">
  <img src="https://github.com/Ssweeties/ENERGY-MONITORING-AND-DEVICE-CONTROL-SYSTEM/blob/367211c614a768c98c3d7ebf71aa34f40d9ed9d1/Diagram.jpg?raw=true" alt="Diagram">
</p>

## Dashboard (Firebase)
*   **Real-time**: View Voltage/Current graphs.
*   **Control**: Toggle switches for Device 1 & 2.

## System Protection
*   **Noise Filtering**: Software filters for ACS712 noise (<50mA) and ZMPT101B (<10V).
*   **Data Safety**: Uses Mutex for thread-safe Firebase operations.
*   **WiFi Reconnect**: Aautomatically attempts to reconnect if signal is lost.

## I/O Summary

| Signal | Type | Description |
| :--- | :--- | :--- |
| `dev1.power` | Output | Calculated Power (W) sent to Cloud/OLED |
| `dev1.energy` | Output | Accumulated Energy (Wh) per session |
| `relay1_on` | Input/Output | Controlled via RFID logic OR Firebase |
| `is_logged_in` | Flag | Global system security state |

## Result Summary
*   **Accurate Metering**: Calibrated for standard AC loads.
*   **Secure**: RFID-based physical access control.
*   **Responsive**: Low-latency control via Firebase Stream.
*   **User-Friendly**: Clear OLED interface for local feedback.

## Author
**Trần Huỳnh**  
Major: Computer Engineering Technology  
Faculty of Electrical-Electronics, HCMUTE  
Email: huynhtran30112004@gmail.com  
