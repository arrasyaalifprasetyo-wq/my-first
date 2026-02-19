#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "time.h"

const char* ssid     = "Arrasya";
const char* password = "kurakura";

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

#define SDA_PIN 4
#define SCL_PIN 5

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// NTP
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7 * 3600;  // WIB
const int   daylightOffset_sec = 0;

void setup() {
  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED gagal");
    while(true);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Connecting WiFi...");
  display.display();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop() {
  struct tm timeinfo;

  if(!getLocalTime(&timeinfo)){
    return;
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("ESP32-C3 CLOCK");

  display.setTextSize(2);
  display.setCursor(0,20);
  display.printf("%02d:%02d:%02d",
                 timeinfo.tm_hour,
                 timeinfo.tm_min,
                 timeinfo.tm_sec);

  display.setTextSize(1);
  display.setCursor(0,50);
  display.printf("%02d-%02d-%04d",
                 timeinfo.tm_mday,
                 timeinfo.tm_mon + 1,
                 timeinfo.tm_year + 1900);

  display.display();

  delay(1000);
}