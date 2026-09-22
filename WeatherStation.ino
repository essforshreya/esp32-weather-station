#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <DHT.h>
#include "config.h"

// OLED (SSD1306 128x64, I2C)
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// DHT11
DHT dht(DHT_PIN, DHT_TYPE);

struct Readings {
  int rawSoil = 0;
  int moisturePercent = 0;

  float temperatureC = NAN;
  float humidityPercent = NAN;
  bool hasValidDht = false;
  bool dhtReadValidThisCycle = false;
};

uint32_t lastReadMs = 0;
Readings readings;

int readSoilRawAveraged(uint8_t samples = 8) {
  uint32_t sum = 0;
  for (uint8_t i = 0; i < samples; i++) {
    sum += analogRead(SOIL_ADC_PIN);
    delay(5);
  }
  return static_cast<int>(sum / samples);
}

int soilRawToPercent(int raw) {
  // Round-based conversion (not map()) per calibration guidance.
  int pct = static_cast<int>(roundf((float)(raw - SOIL_DRY_ADC) * 100.0f / (SOIL_WET_ADC - SOIL_DRY_ADC)));
  return constrain(pct, 0, 100);
}

bool isWateringNeeded(const Readings& r) {
  const bool soilTooDry = (r.moisturePercent < SOIL_MOISTURE_THRESHOLD_PERCENT);
  const bool hotAndDryAir = r.hasValidDht &&
                            (r.temperatureC >= TEMP_HIGH_C) &&
                            (r.humidityPercent <= HUMIDITY_LOW_PERCENT);
  return soilTooDry || hotAndDryAir;
}

void printSerial(const Readings& r, bool wateringNeeded) {
  Serial.println(F("------ ENVIRONMENT ------"));
  if (r.hasValidDht) {
    Serial.printf("Temperature: %d C\n", (int)roundf(r.temperatureC));
    Serial.printf("Humidity   : %d %%\n", (int)roundf(r.humidityPercent));
  } else {
    Serial.println(F("Temperature: N/A"));
    Serial.println(F("Humidity   : N/A"));
  }

  Serial.printf("Soil Raw   : %d\n", r.rawSoil);
  Serial.printf("Moisture   : %d %%\n", r.moisturePercent);

  if (!r.dhtReadValidThisCycle) {
    Serial.println(F("DHT Status : Invalid read (keeping last valid values)"));
  } else {
    Serial.println(F("DHT Status : OK"));
  }

  Serial.printf("WATERING   : %s\n", wateringNeeded ? "NEEDED" : "OK");
  Serial.println();
}

void drawDisplay(const Readings& r, bool wateringNeeded) {
  char line[32];

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tr);

  if (r.hasValidDht) {
    snprintf(line, sizeof(line), "Temp: %d C", (int)roundf(r.temperatureC));
  } else {
    snprintf(line, sizeof(line), "Temp: --");
  }
  u8g2.drawStr(0, 10, line);

  if (r.hasValidDht) {
    snprintf(line, sizeof(line), "Hum : %d %%", (int)roundf(r.humidityPercent));
  } else {
    snprintf(line, sizeof(line), "Hum : --");
  }
  u8g2.drawStr(0, 22, line);

  snprintf(line, sizeof(line), "Soil raw: %d", r.rawSoil);
  u8g2.drawStr(0, 34, line);

  snprintf(line, sizeof(line), "Moisture: %d %%", r.moisturePercent);
  u8g2.drawStr(0, 46, line);

  u8g2.drawStr(0, 58, wateringNeeded ? "WATERING: NEEDED" : "WATERING: OK");
  u8g2.sendBuffer();
}

void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(SOIL_ADC_PIN, INPUT);
  analogReadResolution(12);
  #ifdef ESP32
    analogSetPinAttenuation(SOIL_ADC_PIN, ADC_11db);
  #endif

  dht.begin();

  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
  u8g2.setI2CAddress(OLED_I2C_ADDR << 1); // U8g2 expects 8-bit I2C address
  u8g2.begin();

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(0, 12, "Starting sensors...");
  u8g2.sendBuffer();

  lastReadMs = millis() - READ_INTERVAL_MS; // trigger immediate first read
}

void loop() {
  const uint32_t now = millis();
  if (now - lastReadMs < READ_INTERVAL_MS) {
    return;
  }
  lastReadMs = now;

  // Soil reading
  readings.rawSoil = readSoilRawAveraged(8);
  readings.moisturePercent = soilRawToPercent(readings.rawSoil);

  // DHT11 reading (robust handling of invalid reads)
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  readings.dhtReadValidThisCycle = !(isnan(t) || isnan(h));

  if (readings.dhtReadValidThisCycle) {
    readings.temperatureC = t;
    readings.humidityPercent = h;
    readings.hasValidDht = true;
  }

  const bool wateringNeeded = isWateringNeeded(readings);

  printSerial(readings, wateringNeeded);
  drawDisplay(readings, wateringNeeded);
}
