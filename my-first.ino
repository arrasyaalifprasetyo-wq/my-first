#include <WiFi.h>

const char* ssid = "Arrasya";
const char* password = "kurakura";

#define LED_PIN 8   // kalau tidak nyala coba 2 atau 10

WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  WiFi.begin(ssid, password);
  Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (!client) return;

  String request = client.readStringUntil('\r');
  client.flush();

  if (request.indexOf("/ON") != -1) {
    digitalWrite(LED_PIN, HIGH);
  }
  if (request.indexOf("/OFF") != -1) {
    digitalWrite(LED_PIN, LOW);
  }

  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println();
  client.println("<h1>ESP32-C3 Control</h1>");
  client.println("<a href=\"/ON\"><button>LED ON</button></a>");
  client.println("<a href=\"/OFF\"><button>LED OFF</button></a>");
  client.stop();
}