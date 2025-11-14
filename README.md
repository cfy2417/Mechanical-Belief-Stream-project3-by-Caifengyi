## 📘 README — TouchDesigner + Arduino Data Visualization Project
## 📌 Overview
"Mechanical Belief Stream" is a project that integrates speculative design, interactive art, and wearable devices. It simulates the process of how future beliefs form as nature gradually evolves into mechanical beings. Through this, I aim to prompt viewers to think about the form of future human spiritual world.
## 🎛 System Architecture
Arduino / ESP32  --->  Serial / WiFi  --->  TouchDesigner  --->  Real-time Visual Output
Hardware Components
FSR402 pressure sensors
Light sensor (LM393 or similar)
Temperature & humidity sensor (DHT11 / DHT22)
pH sensor (PH4502C)
Microphone (INMP441 or analog mic)
ESP32 / Arduino board
Optional display: ST7735 / OLED screen

## 📡 Data Transmission
Arduino → TouchDesigner
Two typical options are supported:
1. Serial Communication (USB)
Arduino sends comma-separated values:
fsr1,fsr2,fsr3,light,temp,humidity,ph,mic
2. WiFi (HTTP / UDP / WebSocket)
ESP32 posts JSON data to a local server or directly to TouchDesigner:
{
  "fsr": [123, 98, 240],
  "light": 703,
  "temp": 25.4,
  "humidity": 60,
  "ph": 7.1,
  "mic": 0.14
}
TouchDesigner receives and parses the values through:
Serial DAT
TCP/IP DAT
WebSocket DAT
JSON DAT

## 🎨 Visual Expression in TouchDesigner
The real-time sensor data drives various visual elements, including:
generative patterns
particle systems
shader-based interactions
color shifts and distortions
physics-influenced motion
Each sensor is mapped to a specific visual attribute:
Sensor	Mapping in TouchDesigner
Pressure (FSR)	Scale, force, intensity
Light	Brightness, color temperature
Temperature	Gradient shift
Humidity	Particle turbulence
pH	Color palette mapping
Microphone	Amplitude-driven motion

## 🛠 Installation
1. Arduino / ESP32
Upload the data-sending firmware to your microcontroller.
Example minimal code:
Serial.print(fsrValue);
Serial.print(",");
Serial.print(lightValue);
Serial.print(",");
Serial.println(tempValue);
2. TouchDesigner
Open the .toe file included in the project.
You may need to:
enable serial input
set correct COM port or network IP
adjust channel mapping
▶️ How to Run
Step 1 — Connect Hardware
Plug in Arduino or power your ESP32.
Step 2 — Launch TouchDesigner
Open the project file:
Project2_TouchDesigner_Visualization.toe
Step 3 — Verify Data Reception
Check:
Serial DAT
TCP/IP DAT
JSON Parse node
Make sure channels are updating.
Step 4 — View Final Visual
The final composition is shown in the OUT_TOP.

## 📦 File Structure
project/
│── Arduino/
│   └── sensor_send.ino
│── TouchDesigner/
│   └── project.toe
│── assets/
│── README.md

## ✨ Features
Real-time sensor data streaming
Modular TouchDesigner network
Extensible for more sensors
Easy mapping through CHOP channels
Designed for interactive installations

## 🧑‍💻 Author
Cai Fengyi
2025
