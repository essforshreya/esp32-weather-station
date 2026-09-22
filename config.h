#pragma once

// ---------------- Pin Map ----------------
#define DHT_PIN         13   // IO13 -> dht_1.DATA
#define SOIL_ADC_PIN    26   // IO26 -> soil_1.AOUT
#define OLED_SDA_PIN    21   // IO21 -> oled_1.SDA
#define OLED_SCL_PIN    22   // IO22 -> oled_1.SCL
#define OLED_I2C_ADDR   0x3C

// ---------------- Sensor Types ----------------
#define DHT_TYPE DHT11

// ---------------- Timing ----------------
static const uint32_t READ_INTERVAL_MS = 2000;   // DHT11 safe interval (>= 2000 ms)

// ---------------- Soil Calibration (ESP32 12-bit ADC) ----------------
// Inverted probe: DRY gives higher ADC, WET gives lower ADC.
static const int SOIL_DRY_ADC = 3723;            // ~3.0V in air
static const int SOIL_WET_ADC = 1489;            // ~1.2V in water

// ---------------- Decision Thresholds ----------------
static const int   SOIL_MOISTURE_THRESHOLD_PERCENT = 35;  // below this => watering needed
static const float TEMP_HIGH_C = 30.0f;                   // high temperature threshold
static const float HUMIDITY_LOW_PERCENT = 40.0f;          // low humidity threshold
