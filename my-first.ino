#include <Wire.h>
#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BMP280.h>
#include <Preferences.h>
#include "time.h"
#include "esp_sleep.h"
#include <math.h>

// ================= OLED =================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ================= I2C PIN ESP32-C3 =================
#define SDA_PIN 2
#define SCL_PIN 0

// ================= TOUCH =================
#define TOUCH_PIN 1
Preferences prefs;

// ================= WIFI =================
const char* ssid = "Arrasya";
const char* password = "kurakura";

// ================= NTP =================
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7 * 3600;
const int daylightOffset_sec = 0;

// ================= BMP280 =================
Adafruit_BMP280 bmp;

// ================= VARIABLE =================
int sweepAngle = 0;
String prevHour = "--";
String prevMin = "--";
String prevSec = "--";

// =================================================
// WIFI + SYNC TIME
// =================================================
void connectWiFiAndSyncTime() {
  display.clearDisplay();
  display.setCursor(0, 25);
  display.setTextSize(1);
  display.print("Connecting WiFi...");
  display.display();

  WiFi.begin(ssid, password);
  int retry = 0;

  while (WiFi.status() != WL_CONNECTED && retry < 20) {
    delay(500);
    display.print(".");
    display.display();
    retry++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    display.clearDisplay();
    display.setCursor(0, 25);
    display.print("Time Synced");
    display.display();
    delay(1000);
  }
}

// =================================================
// GET TIME
// =================================================
String getTime(char formatType) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "--";

  char buffer[8];
  if (formatType == 'H') strftime(buffer, sizeof(buffer), "%H", &timeinfo);
  if (formatType == 'M') strftime(buffer, sizeof(buffer), "%M", &timeinfo);
  if (formatType == 'S') strftime(buffer, sizeof(buffer), "%S", &timeinfo);

  return String(buffer);
}

// =================================================
// RADAR BASE
// =================================================
void drawRadarBase() {
  int cx = 96;
  int cy = 40;

  display.drawCircle(cx, cy, 20, WHITE);
  display.drawCircle(cx, cy, 15, WHITE);
  display.drawCircle(cx, cy, 10, WHITE);
}

// =================================================
// RADAR SWEEP
// =================================================
void drawRadarSweep() {
  int cx = 96;
  int cy = 40;

  float rad = sweepAngle * 3.14159 / 180.0;
  int x2 = cx + cos(rad) * 20;
  int y2 = cy + sin(rad) * 20;

  display.drawLine(cx, cy, x2, y2, WHITE);
  display.display();
  delay(15);
  display.drawLine(cx, cy, x2, y2, BLACK);

  sweepAngle += 5;
  if (sweepAngle >= 360) sweepAngle = 0;
}

// =================================================
// DRAW CLOCK
// =================================================
void drawClock(String h, String m) {
  if (h != prevHour || m != prevMin) {
    prevHour = h;
    prevMin = m;

    display.fillRect(0, 0, 70, 16, BLACK);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print(h + ":" + m);
    display.display();
  }
}

// =================================================
// DRAW SECONDS
// =================================================
void drawSeconds(String s) {
  if (s != prevSec) {
    prevSec = s;

    display.fillRect(0, 18, 60, 10, BLACK);
    display.setTextSize(1);
    display.setCursor(0, 18);
    display.print("Sec: " + s);
    display.display();
  }
}

// =================================================
// DRAW BMP280
// =================================================
void drawEnv(float temp, float pressure) {
  display.fillRect(0, 30, 80, 30, BLACK);

  display.setCursor(0, 30);
  display.print("T:");
  display.print(temp, 1);
  display.print("C");

  display.setCursor(0, 42);
  display.print("P:");
  display.print(pressure / 100.0, 1);
  display.print("hPa");

  display.display();
}

// =================================================
// SLEEP MODE
// =================================================
void goToSleep() {
  display.clearDisplay();
  display.setCursor(20, 30);
  display.print("Sleep Mode");
  display.display();
  delay(1000);

  pinMode(TOUCH_PIN, INPUT_PULLDOWN);
  esp_deep_sleep_enable_gpio_wakeup(1ULL << TOUCH_PIN, ESP_GPIO_WAKEUP_GPIO_HIGH);

  esp_deep_sleep_start();
}

// =================================================
// SETUP
// =================================================
void setup() {

  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (true);
  }

  display.clearDisplay();
  display.setTextColor(WHITE);

  if (!bmp.begin(0x76)) {
    display.setCursor(0, 25);
    display.print("BMP280 Error");
    display.display();
    while (true);
  }

  connectWiFiAndSyncTime();
}

// =================================================
// LOOP
// =================================================
void loop() {

  String h = getTime('H');
  String m = getTime('M');
  String s = getTime('S');

  float temp = bmp.readTemperature();
  float pressure = bmp.readPressure();

  drawClock(h, m);
  drawSeconds(s);
  drawEnv(temp, pressure);
  drawRadarBase();
  drawRadarSweep();

  if (digitalRead(TOUCH_PIN) == HIGH) {
    goToSleep();
  }
}