# Smart Medical Infusion Pump — IoT Monitoring & Control System

A complete end-to-end IoT platform for remote monitoring and control of the **ESP32-S3 Medical Syringe Infusion Pump**.

---

## 🏗️ System Components

1. **Edge Device (ESP32-S3 Dual Core N16R8)**
   - **Core 1 (Deterministic Motion & Safety)**: Hardware timer ISR step generator, HX711 load-cell occlusion detection, VL53L0X Time-of-Flight travel sensor, limit switches, and ILI9341 SPI color TFT display.
   - **Core 0 (Networking & Telemetry)**: Non-blocking Wi-Fi stack, MQTT client publishing 5 Hz telemetry and instant occlusion alarms to `pumps/{id}/telemetry` and `pumps/{id}/alarms`.

2. **Backend Platform (Dockerized)**
   - **PostgreSQL 16**: Time-series telemetry logs, device registries, and occlusion event audit trails.
   - **Eclipse Mosquitto**: Lightweight MQTT broker (ports `1883` and `9001`).
   - **FastAPI (Python)**: REST endpoints + WebSocket streaming server bridging MQTT to connected web dashboards.

3. **Frontend Dashboard (React + TypeScript + Tailwind)**
   - Live real-time dashboard displaying occlusion force, plunger travel, step rate, and safety alarms with remote Pause/Infuse/Retract controls.

---

## 🚀 Quick Start Guide

### Step 1: Start Backend & Infrastructure with Docker

```bash
cd docker
docker compose up -d --build
```

Verify containers are running:
```bash
docker compose ps
```
- PostgreSQL: `localhost:5432`
- Mosquitto MQTT: `localhost:1883` (TCP) and `9001` (WebSocket)
- FastAPI Gateway: `http://localhost:8000` (Swagger docs at `http://localhost:8000/docs`)

---

### Step 2: Configure & Flash ESP32-S3 Firmware

1. Open `firmware/` in **PlatformIO** (VS Code) or configure in **Arduino IDE**.
2. Open `firmware/include/config.h` and update:
   ```c
   #define WIFI_SSID         "Your_WiFi_SSID"
   #define WIFI_PASSWORD     "Your_WiFi_Password"
   #define MQTT_BROKER_HOST  "192.168.1.xxx"  // IP address of computer running Docker
   ```
3. Upload to your **ESP32-S3-DevKitC-1 (N16R8)**.
4. Open the Serial Monitor at `115200 baud` to observe Wi-Fi connection and telemetry streaming.

---

### Step 3: Run the Web Dashboard

```bash
cd frontend
npm install
npm run dev
```
Open `http://localhost:5173` in your browser. Telemetry from your ESP32-S3 will stream into the dashboard in real time!

