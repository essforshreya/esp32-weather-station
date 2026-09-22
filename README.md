# ESP32 Weather Station

An ESP32-based environmental monitoring system that measures
temperature, humidity, and soil moisture, displays the readings
on an OLED, and determines whether plant watering is required.

## Current Features

- Real-time temperature measurement using DHT11
- Real-time humidity measurement using DHT11
- Capacitive soil-moisture measurement
- Soil-moisture percentage calibration
- 8-sample averaging for soil readings
- SSD1306 128x64 OLED display
- Serial monitor output
- DHT11 invalid-read handling
- Watering-needed decision logic

## Hardware

- ESP32 DevKit V1
- DHT11
- Capacitive Soil Moisture Sensor v1.2
- SSD1306 OLED 128x64 I2C
- Breadboard
- Jumper wires

## Pin Configuration

| Component | ESP32 |
|---|---|
| DHT11 DATA | GPIO 13 |
| Soil Sensor AOUT | GPIO 26 |
| OLED SDA | GPIO 21 |
| OLED SCL | GPIO 22 |
| OLED VCC | 3.3V |
| OLED GND | GND |

## Soil Moisture Calibration

The capacitive soil-moisture sensor uses an inverted ADC relationship:

- Dry condition → higher ADC value
- Wet condition → lower ADC value

Current calibration:

- Dry: 3723 ADC
- Wet: 1489 ADC

The ESP32 uses a 12-bit ADC.

## Watering Logic

Watering is marked as required when:

1. Soil moisture falls below 35%

OR

2. Temperature is at least 30°C
   AND humidity is at most 40%

## Software

- Arduino IDE
- C/C++
- ESP32 Arduino Core
- DHT library
- U8g2 library
- Wire library

## Current Status

### Version 1 — Local Monitoring

- [x] Sensor data acquisition
- [x] Soil calibration
- [x] OLED display
- [x] Serial monitoring
- [x] Watering decision logic

### Version 2 — Cloud Monitoring

- [ ] Wi-Fi connectivity
- [ ] Cloud data storage
- [ ] Remote dashboard
- [ ] Historical data visualization

### Future Improvements

- Cloud-based environmental monitoring
- Historical climate graphs
- Remote alerts
- Weather API integration
- Irrigation recommendation system
- Long-term environmental data analysis
