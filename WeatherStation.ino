#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <DHT.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "config.h"
#include "secrets.h"

// ---------------- OLED ----------------
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(
  U8G2_R0,
  U8X8_PIN_NONE
);

// ---------------- DHT11 ----------------
DHT dht(DHT_PIN, DHT_TYPE);

// ---------------- Readings ----------------
struct Readings {
  int rawSoil = 0;
  int moisturePercent = 0;

  float temperatureC = NAN;
  float humidityPercent = NAN;

  bool hasValidDht = false;
  bool dhtReadValidThisCycle = false;
};

Readings readings;

uint32_t lastReadMs = 0;
uint32_t lastCloudUpdateMs = 0;

// ThingSpeak free accounts should not be updated more frequently
// than once every 15 seconds.
static const uint32_t CLOUD_UPDATE_INTERVAL_MS = 15000;


// ============================================================
// SOIL SENSOR
// ============================================================

int readSoilRawAveraged(uint8_t samples = 8) {
  uint32_t sum = 0;

  for (uint8_t i = 0; i < samples; i++) {
    sum += analogRead(SOIL_ADC_PIN);
    delay(5);
  }

  return static_cast<int>(sum / samples);
}


int soilRawToPercent(int raw) {
  int pct = static_cast<int>(
    roundf(
      (float)(raw - SOIL_DRY_ADC) * 100.0f /
      (SOIL_WET_ADC - SOIL_DRY_ADC)
    )
  );

  return constrain(pct, 0, 100);
}


// ============================================================
// WATERING DECISION
// ============================================================

bool isWateringNeeded(const Readings& r) {

  const bool soilTooDry =
    (r.moisturePercent < SOIL_MOISTURE_THRESHOLD_PERCENT);

  const bool hotAndDryAir =
    r.hasValidDht &&
    (r.temperatureC >= TEMP_HIGH_C) &&
    (r.humidityPercent <= HUMIDITY_LOW_PERCENT);

  return soilTooDry || hotAndDryAir;
}


// ============================================================
// WIFI
// ============================================================

void connectWiFi() {

  Serial.println();
  Serial.print("Connecting to Wi-Fi");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;

  while (WiFi.status() != WL_CONNECTED && attempts < 30) {

    delay(500);
    Serial.print(".");

    attempts++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("Wi-Fi connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

  } else {

    Serial.println("Wi-Fi connection failed.");
  }
}


// ============================================================
// THINGSPEAK
// ============================================================

void uploadToThingSpeak(const Readings& r, bool wateringNeeded) {

  if (WiFi.status() != WL_CONNECTED) {

    Serial.println("Wi-Fi disconnected. Trying to reconnect...");
    connectWiFi();

    if (WiFi.status() != WL_CONNECTED) {
      return;
    }
  }

  HTTPClient http;

  String url = "http://api.thingspeak.com/update?api_key=";
  url += THINGSPEAK_API_KEY;

  // Field 1 = Temperature
  // Field 2 = Humidity
  // Field 3 = Soil Moisture
  // Field 4 = Watering Required

  if (r.hasValidDht) {
    url += "&field1=";
    url += String(r.temperatureC, 2);

    url += "&field2=";
    url += String(r.humidityPercent, 2);
  }

  url += "&field3=";
  url += String(r.moisturePercent);

  url += "&field4=";
  url += String(wateringNeeded ? 1 : 0);

  http.begin(url);

  int httpCode = http.GET();

  if (httpCode > 0) {

    Serial.print("ThingSpeak HTTP response: ");
    Serial.println(httpCode);

    String response = http.getString();

    Serial.print("ThingSpeak response: ");
    Serial.println(response);

    if (httpCode == 200) {
      Serial.println("Cloud update successful!");
    }

  } else {

    Serial.print("ThingSpeak upload failed: ");
    Serial.println(http.errorToString(httpCode));
  }

  http.end();
}


// ============================================================
// SERIAL OUTPUT
// ============================================================

void printSerial(
  const Readings& r,
  bool wateringNeeded
) {

  Serial.println(F("------ ENVIRONMENT ------"));

  if (r.hasValidDht) {

    Serial.printf(
      "Temperature: %d C\n",
      (int)roundf(r.temperatureC)
    );

    Serial.printf(
      "Humidity   : %d %%\n",
      (int)roundf(r.humidityPercent)
    );

  } else {

    Serial.println(F("Temperature: N/A"));
    Serial.println(F("Humidity   : N/A"));
  }

  Serial.printf(
    "Soil Raw   : %d\n",
    r.rawSoil
  );

  Serial.printf(
    "Moisture   : %d %%\n",
    r.moisturePercent
  );

  if (!r.dhtReadValidThisCycle) {

    Serial.println(
      F("DHT Status : Invalid read (keeping last valid values)")
    );

  } else {

    Serial.println(F("DHT Status : OK"));
  }

  Serial.printf(
    "WATERING   : %s\n",
    wateringNeeded ? "NEEDED" : "OK"
  );

  Serial.println();
}


// ============================================================
// OLED DISPLAY
// ============================================================

void drawDisplay(
  const Readings& r,
  bool wateringNeeded
) {

  char line[32];

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tr);

  if (r.hasValidDht) {

    snprintf(
      line,
      sizeof(line),
      "Temp: %d C",
      (int)roundf(r.temperatureC)
    );

  } else {

    snprintf(line, sizeof(line), "Temp: --");
  }

  u8g2.drawStr(0, 10, line);


  if (r.hasValidDht) {

    snprintf(
      line,
      sizeof(line),
      "Hum : %d %%",
      (int)roundf(r.humidityPercent)
    );

  } else {

    snprintf(line, sizeof(line), "Hum : --");
  }

  u8g2.drawStr(0, 22, line);


  snprintf(
    line,
    sizeof(line),
    "Soil raw: %d",
    r.rawSoil
  );

  u8g2.drawStr(0, 34, line);


  snprintf(
    line,
    sizeof(line),
    "Moisture: %d %%",
    r.moisturePercent
  );

  u8g2.drawStr(0, 46, line);


  u8g2.drawStr(
    0,
    58,
    wateringNeeded
      ? "WATERING: NEEDED"
      : "WATERING: OK"
  );

  u8g2.sendBuffer();
}


// ============================================================
// SETUP
// ============================================================

void setup() {

  Serial.begin(115200);

  delay(100);


  // ---------- Soil sensor ----------

  pinMode(SOIL_ADC_PIN, INPUT);

  analogReadResolution(12);

  #ifdef ESP32
    analogSetPinAttenuation(
      SOIL_ADC_PIN,
      ADC_11db
    );
  #endif


  // ---------- DHT ----------

  dht.begin();


  // ---------- OLED ----------

  Wire.begin(
    OLED_SDA_PIN,
    OLED_SCL_PIN
  );

  u8g2.setI2CAddress(
    OLED_I2C_ADDR << 1
  );

  u8g2.begin();


  u8g2.clearBuffer();

  u8g2.setFont(
    u8g2_font_6x12_tr
  );

  u8g2.drawStr(
    0,
    12,
    "Starting sensors..."
  );

  u8g2.sendBuffer();


  // ---------- Wi-Fi ----------

  connectWiFi();


  // Make first sensor reading happen immediately
  lastReadMs =
    millis() - READ_INTERVAL_MS;

  // Allow first cloud update immediately
  lastCloudUpdateMs =
    millis() - CLOUD_UPDATE_INTERVAL_MS;
}


// ============================================================
// LOOP
// ============================================================

void loop() {

  const uint32_t now = millis();


  // ==========================================================
  // SENSOR READING EVERY 2 SECONDS
  // ==========================================================

  if (now - lastReadMs >= READ_INTERVAL_MS) {

    lastReadMs = now;


    // ---------- Soil ----------

    readings.rawSoil =
      readSoilRawAveraged(8);

    readings.moisturePercent =
      soilRawToPercent(
        readings.rawSoil
      );


    // ---------- DHT11 ----------

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    readings.dhtReadValidThisCycle =
      !(isnan(t) || isnan(h));


    if (readings.dhtReadValidThisCycle) {

      readings.temperatureC = t;
      readings.humidityPercent = h;

      readings.hasValidDht = true;
    }


    // ---------- Decision ----------

    const bool wateringNeeded =
      isWateringNeeded(readings);


    // ---------- Local output ----------

    printSerial(
      readings,
      wateringNeeded
    );

    drawDisplay(
      readings,
      wateringNeeded
    );
  }


  // ==========================================================
  // CLOUD UPDATE EVERY 15 SECONDS
  // ==========================================================

  if (
    now - lastCloudUpdateMs >=
    CLOUD_UPDATE_INTERVAL_MS
  ) {

    lastCloudUpdateMs = now;

    const bool wateringNeeded =
      isWateringNeeded(readings);

    uploadToThingSpeak(
      readings,
      wateringNeeded
    );
  }
}
