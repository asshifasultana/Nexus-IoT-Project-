# 🔷 NEXUS: Advanced IoT Safety & Surveillance Hub

**NEXUS** is a multi-sensor, ESP32-based safety system designed for real-time environmental monitoring, automated hazard response, and remote surveillance. It integrates fire detection, gas sensing, motion detection, and live video streaming into a unified security ecosystem.

---

## 🚀 Features

### 📡 Real-Time Multi-Sensor Monitoring
* **PIR (Human Motion)**: Detects unauthorized movement in restricted areas.
* **MQ-2 (Smoke/Gas)**: Monitors for smoke or combustible gas leaks.
* **Vibration Sensor**: Detects physical tampering or environmental tremors (Earthquake).
* **Dual IR Flame Sensors**: Specialized infrared detection for a tiered fire response.

### 🔥 Tiered Fire Response System
* **Tier 1 (Early Warning)**: Triggered by the primary Flame Sensor or Smoke Detector → Sends an immediate **Telegram alert** and sounds the buzzer.
* **Tier 2 (Hardware-Linked Suppression)**: The secondary Flame Sensor is hardwired directly to the **Relay Module** (D0 to IN) → Activates the **water pump** instantly, bypassing software latency for maximum reliability.

### 📸 Live Surveillance
* **ESP32-CAM**: Streams live video over a local IP address.
* **Cloud Integration**: The stream link is automatically pushed to your Telegram bot upon system startup or hazard detection.

### ⚡ Robust Power & Fail-Safe Design
* **Isolated Rails**: Logic runs on 5V USB, while the Pump/Relay uses a 7.4V battery to prevent brownouts.
* **Hardware Interlock**: The fire suppression system remains functional even if the microcontroller loses WiFi or crashes.

---

## 🛠️ Hardware Requirements

| Component | Specification |
| :--- | :--- |
| **Main Controller** | ESP32 DevKit V1 |
| **Camera Module** | ESP32-CAM |
| **Fire Sensors** | 2× IR Flame Sensors (Digital Output) |
| **Air Quality** | MQ-2 Smoke/Gas Sensor |
| **Motion/Impact** | PIR Sensor & Vibration Sensor |
| **Switching** | 5V Relay Module (Active-Low) |
| **Suppression** | Submersible Water Pump |
| **Alerting** | Active Piezo Buzzer |
| **Power Supply** | 7.4V Battery (Motor) + 5V USB (Logic) |

---

## 🔌 Pin Configuration

| Component | ESP32 Pin (GPIO) | Connection Note |
| :--- | :--- | :--- |
| **MQ-2 Smoke Sensor** | GPIO 34 | Analog Input |
| **PIR Motion Sensor** | GPIO 13 | Digital Input |
| **Vibration Sensor** | GPIO 14 | Digital Input |
| **Flame Sensor 1** | GPIO 27 | Digital Input (Alert Only) |
| **Flame Sensor 2** | GPIO 26 | **Monitoring Input** (Hardwired to Relay IN) |
| **Buzzer** | GPIO 12 | Digital Output |
| **Relay Module** | -- | Hardwired to Flame Sensor 2 D0 |

---

## 📡 System Architecture

1.  **Sense**: NEXUS polls the PIR, MQ-2, Vibration, and Flame sensors.
2.  **Warn**: Local buzzer activates for any hazard (Fire, Motion, or Earthquake).
3.  **Notify**: Telegram bot dispatches detailed alerts (e.g., "🚨 EARTHQUAKE DETECTED") with the live camera IP.
4.  **Respond**: The hardware-level link between the flame sensor and relay ensures the water pump activates the moment fire is confirmed.

---

## ⚙️ Setup & Installation

### 1. Arduino IDE Requirements
Install ESP32 board support and the following libraries:
- `WiFi.h`
- `WiFiClientSecure.h`
- `UniversalTelegramBot`
- `ArduinoJson`

### 2. Configure Credentials
Update your credentials in the `NEXUS` source code:
```cpp
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";
#define BOT_TOKEN "YOUR_BOT_TOKEN"
#define CHAT_ID "YOUR_CHAT_ID"
