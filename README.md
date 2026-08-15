# 🦯 Smart Blind Navigation Stick

<p align="center">

[![Platform](https://img.shields.io/badge/Platform-ESP32%20%7C%20Arduino%20Nano-blue?style=for-the-badge)]()
[![Language](https://img.shields.io/badge/Language-C%2FC%2B%2B-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)]()
[![IoT](https://img.shields.io/badge/IoT-ThingSpeak-00A86B?style=for-the-badge)](https://thingspeak.com/)
[![GPS](https://img.shields.io/badge/GPS-NEO--6M-orange?style=for-the-badge)]()
[![GSM](https://img.shields.io/badge/GSM-SIM800L-red?style=for-the-badge)]()
[![License](https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge)](LICENSE)

</p>

---

> A low-cost IoT assistive device combining real-time obstacle detection, GPS tracking, emergency communication, and remote monitoring.

---

## 📌 Project Overview

The **IoT Blind Assistive Stick** is a low-cost, dual-microcontroller assistive device for visually impaired individuals. It provides real-time obstacle detection, GPS location tracking, emergency alerts via SMS and call, and a live location dashboard.

**Total Hardware Cost:** ~₹1500 (including cane and wiring)

---

## ✨ Features

| Feature | Status |
|---------|--------|
| Upper Obstacle Detection (100-50cm → Slow beep) | ✅ |
| Upper Obstacle Detection (50-0cm → Fast beep) | ✅ |
| Lower Obstacle Detection (<50cm → Double pulse) | ✅ |
| Pit Detection (Sudden ground drop → Continuous alert) | ✅ |
| GPS Location Tracking (NEO-6M) | ✅ |
| SOS Button (SMS + Call to 4 numbers) | ✅ |
| GPRS Data Upload to ThingSpeak | ✅ |
| Live Location on Map (Google Maps) | ✅ |
| Battery Health Monitoring (2 batteries, average %) | ✅ |
| Offline Location Cache | ✅ |

---

## 🏗️ System Architecture

```text
┌─────────────────────────────────────────────────────────────────────┐
│                         SMART BLIND STICK                           │
├──────────────────────────────┬──────────────────────────────────────┤
│         ARDUINO NANO         │                ESP32                 │
│       Real-time Control      │         Communication Hub            │
├──────────────────────────────┼──────────────────────────────────────┤
│ • Upper Ultrasonic (D2/D3)   │ • GPS (GPIO 16/17)                  │
│ • Lower Ultrasonic (D4/D5)   │ • GSM (GPIO 26/27)                  │
│ • Buzzer (D6)                │ • SOS Button (GPIO 5)               │
│ • Distance-based Alerts      │ • Battery ADC (GPIO 34/35)          │
│ • Pit Detection Logic        │ • ThingSpeak via GPRS              │
└──────────────────────────────┴──────────────────────────────────────┘
                              │
                              │ GPRS / 2G Data
                              ▼
                    ┌──────────────────────┐
                    │      ThingSpeak      │
                    │      Cloud DB        │
                    └──────────┬───────────┘
                               │
                               │ HTTP
                               ▼
                    ┌──────────────────────┐
                    │   Custom Dashboard   │
                    │                      │
                    │ • Live Map           │
                    │ • Battery %          │
                    └──────────────────────┘
```
---

## 🔧 Hardware Components

| Component | Quantity | Purpose |
|-----------|----------|---------|
| ESP32-WROOM Dev Board | 1 | Main communication hub |
| Arduino Nano | 1 | Real-time obstacle detection |
| HC-SR04 Ultrasonic Sensor | 2 | Upper & lower obstacle detection |
| NEO-6M GPS Module | 1 | Location tracking |
| SIM800L GSM Module | 1 | SMS, Call, GPRS data |
| 18650 Battery | 2 | Power (1 for GSM, 1 for rest) |
| 1000uF Capacitor | 1 | GSM power smoothing |
| 100kΩ Resistor | 4 | Voltage divider for battery |
| Buzzer (5V) | 1 | Audio alerts |
| Push Button | 1 | SOS trigger |
| Cane | 1 | Physical structure |

> **Full component details with wiring:** See [docs/components-list.txt](docs/components-list.txt)


---

## 🔌 Pin Connections

### ESP32 Connections

| ESP32 Pin | Connected To | Purpose |
|-----------|--------------|---------|
| GPIO16 | GPS TX | GPS Data Receive |
| GPIO17 | GPS RX | GPS Command Send |
| GPIO26 | GSM TX | GSM Command Send |
| GPIO27 | GSM RX | GSM Data Receive |
| GPIO5 | SOS Button | Emergency Trigger |
| GPIO34 | Battery 1 ADC | Voltage Monitoring |
| GPIO35 | Battery 2 ADC | Voltage Monitoring |

### Arduino Nano Connections

| Nano Pin | Connected To | Purpose |
|----------|--------------|---------|
| D2 | Upper Ultrasonic TRIG | Obstacle detection |
| D3 | Upper Ultrasonic ECHO | Obstacle detection |
| D4 | Lower Ultrasonic TRIG | Ground detection |
| D5 | Lower Ultrasonic ECHO | Ground detection |
| D6 | Buzzer | Audio alert |

---

## 📂 Code Structure

```text
smart-blind-stick/
│
├── README.md
├── LICENSE
├── .gitignore
├── esp32_config_template.h
│
├── firmware/
│   ├── esp32_code/
│   │   └── esp32_main.ino
│   │
│   └── nano_code/
│       └── nano_obstacle.ino
│
├── dashboard/
│   ├── index.html
│   ├── style.css
│   └── script.js
│
├── docs/
│   └── components-list.txt
│
└── media/
    ├── hardware-photo.jpg
    ├── demo-video.mp4
    └── dashboard-screenshot.jpg
```
---

## 🚀 Installation & Setup

### Prerequisites

- [Arduino IDE](https://www.arduino.cc/en/software)
- [ESP32 Board Support](https://github.com/espressif/arduino-esp32)
- Required Libraries:
  - `TinyGPSPlus` (GPS parsing)
  - `TinyGsmClient` (GSM/GPRS)

### Step 1: Configure Credentials

1. Copy `esp32_config_template.h` to `esp32_config.h`
2. Fill in your details:
   - ThingSpeak API Key
   - Emergency phone numbers (4 numbers)
   - GPRS APN (for your SIM)

### Step 2: Upload Arduino Nano Code

1. Open `firmware/nano_code/nano_obstacle.ino`
2. Select Board: **Arduino Nano**
3. Select Port: (your Nano COM port)
4. Click **Upload**

### Step 3: Upload ESP32 Code

1. Open `firmware/esp32_code/esp32_main.ino`
2. Select Board: **ESP32 Dev Module**
3. Select Port: (your ESP32 COM port)
4. Click **Upload**

### Step 4: Power the System

| Battery | Powers |
|---------|--------|
| Battery 1 (18650) | SIM800L (DIRECT - no converter) |
| Battery 2 (18650) | ESP32 + Arduino Nano + Sensors (via converter) |

> **Critical:** SIM800L MUST connect directly to battery. Do NOT use boost/buck converter for GSM.

### Step 5: Deploy Dashboard

1. Upload `dashboard/` files to any static hosting:
   - GitHub Pages (free)
   - Local browser (open index.html)
2. Dashboard reads data from ThingSpeak and displays on map

---

## 📊 Alert Patterns (Arduino Nano)

| Condition | Sound Pattern | Behavior |
|-----------|---------------|----------|
| Front 51-100cm | Sound 1 | 300ms ON, 300ms OFF |
| Front <50cm | Sound 2 | 100ms ON, 100ms OFF |
| Lower <20cm | Sound 3 | Double pulse (80ms ON, 80ms OFF, 80ms ON) |
| Pit detected | Sound 4 | Continuous ON (highest priority) |
| Both sensors <20cm | Sound 5 | Custom pattern |

---

## 🆘 Emergency SOS Functionality

When SOS button is pressed, ESP32:

1. **Waits for GPS fix** (up to 30 seconds)
2. **Sends SMS** to all 4 emergency contacts with Google Maps link
3. **Calls** each number (rings for 15 seconds, then hangs up)
4. **Uploads** location to ThingSpeak for dashboard display

**SMS Format:**
🚨 EMERGENCY! Smart blind stick activated.
Location: https://maps.google.com/?q=lat,lon
Average Battery: XX%

text

---

## 📡 ThingSpeak Integration

ESP32 sends the following data every 15 seconds via GPRS:

| Field | Data |
|-------|------|
| Field 1 | Latitude (6 decimal places) |
| Field 2 | Longitude (6 decimal places) |
| Field 3 | Average Battery Percentage |

**Dashboard reads these fields and displays location on map.**

---

## ⚠️ Known Limitations

| Issue | Explanation |
|-------|-------------|
| **2G Network Required** | SIM800L is 2G-only. Works with VI/Airtel SIM. Jio not supported. |
| **GPS Fix Time** | First fix takes 30-60 seconds outdoors. Needs clear sky. |
| **GSM Power Spikes** | SIM800L draws 2A peaks. Requires dedicated battery + 1000uF capacitor. |

---

## 🔧 Troubleshooting

| Problem | Solution |
|---------|----------|
| GSM not connecting | Check 2G availability (VI/Airtel only). Add 1000uF capacitor. |
| GPS no fix | Go outdoors. Wait 2-3 minutes. Check antenna. |
| ESP32 resets during GSM | Separate battery for GSM. Add capacitor. Use thicker wires. |
| False ultrasonic readings | Add delay between readings. Implement median filter. |

> **Detailed troubleshooting:** See [docs/components-list.txt](docs/components-list.txt)

---

## 🔮 Future Improvements

- [ ] Upgrade to 4G module (SIM7600) for better coverage
- [ ] Add fall detection (MPU6050 accelerometer)
- [ ] Voice feedback (Text-to-Speech)
- [ ] Water detection sensor
- [ ] Custom PCB design

---

## 📝 License

Distributed under the MIT License. See `LICENSE` for more information.

---

## 👤 Author

**Sunil Mohanty**

- GitHub: [@sunilmohanty23](https://github.com/sunilmohanty23)
- LinkedIn: [Sunil Mohanty](https://www.linkedin.com/in/sunil-mohanty5)

---

## 🙏 Acknowledgments

- TinyGPS++ library by Mikal Hart
- TinyGsmClient library by Volodymyr Shymanskyy
- ThingSpeak for free IoT backend
- Arduino & ESP32 communities

---

## ⭐ Show Your Support

If this project helped you, please give it a ⭐ on GitHub!

---

## 📧 Contact

For questions or suggestions, please open an issue on GitHub.
