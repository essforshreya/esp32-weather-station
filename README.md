# ESP32 Weather Station with Cloud Monitoring 🌦️

An ESP32-based environmental monitoring system that measures temperature, humidity, and soil moisture, displays readings locally on an OLED, and uploads sensor data to ThingSpeak for cloud-based monitoring and visualization.

## 🚀 Features

- 🌡️ Temperature monitoring using DHT11
- 💧 Humidity monitoring using DHT11
- 🌱 Soil moisture monitoring using a capacitive soil moisture sensor
- 🖥️ Real-time local display using SSD1306 OLED
- 🚿 Watering-needed detection based on soil moisture and environmental conditions
- 📡 Wi-Fi connectivity using ESP32
- ☁️ Cloud data logging using ThingSpeak
- 📊 Historical sensor data visualization through ThingSpeak
- 🔒 Separate credentials file to keep Wi-Fi and API keys private

## 🛠️ Hardware Used

- ESP32 DevKit V1
- DHT11 Temperature & Humidity Sensor
- Capacitive Soil Moisture Sensor v1.2
- SSD1306 128×64 OLED Display
- Breadboard
- Jumper wires
- USB cable

## 🔌 Pin Connections

| Component | Pin | ESP32 |
|---|---|---|
| DHT11 | DATA | GPIO 13 |
| Soil Moisture Sensor | AOUT | GPIO 26 |
| OLED | SDA | GPIO 21 |
| OLED | SCL | GPIO 22 |
| OLED | VCC | 3.3V |
| OLED | GND | GND |
| DHT11 | VCC | 3.3V |
| DHT11 | GND | GND |
| Soil Sensor | VCC | 3.3V |
| Soil Sensor | GND | GND |

## ☁️ Cloud Monitoring

Sensor readings are periodically uploaded to a ThingSpeak channel using Wi-Fi.

### ThingSpeak Fields

| Field | Parameter |
|---|---|
| Field 1 | Temperature (°C) |
| Field 2 | Humidity (%) |
| Field 3 | Soil Moisture (%) |
| Field 4 | Watering Required |

The cloud dashboard provides historical graphs that can be used to observe environmental changes over time.

## ⚙️ Software & Libraries

- Arduino IDE
- ESP32 Arduino Core
- C/C++
- U8g2
- DHT sensor library
- WiFi
- HTTPClient
- ThingSpeak

## 📁 Project Structure

```text
esp32-weather-station/
│
├── WeatherStation.ino
├── config.h
├── secrets.example.h
├── .gitignore
└── README.md
