#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include <TFT_eSPI.h>
#include <qrcode_espi.h>
#include <WiFi.h>
#include <HTTPClient.h>
// 0. TOUCH SCREEN PINS
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25
#define XPT2046_CS 33
// 1. WiFi Global Setup
const char* ssid = "Chanel Ultimate Edition";
const char* password = "48590413";
const char* qrDataUrl = "http://192.168.4.143:8000/getIdentity";
const char* openDoorUrl = "http://192.168.4.143:8000/openDoor";
// 2. SPI Global Setup
SPIClass mySpi = SPIClass(VSPI);
// 3. XPT2046 Touchscreen Global Setup
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);
// 4. TFT_eSPI Global Setup
TFT_eSPI tft = TFT_eSPI();
// 5. QRCode Global Setup
QRcode_eSPI qrcode (&tft);
// 6. Prototypes
void openDoorRequest();
void getCodeRequest();

void setup() {
  // 1. Serial Setup
  Serial.begin(115200);
  // 2. SPI Setup
  mySpi.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  ts.begin(mySpi);
  ts.setRotation(1);
  // 3. TFT_eSPI Setup
  tft.init();
  tft.setRotation(1);
  tft.setSwapBytes(true);
  tft.setTextFont(2);
  // 4. WiFi Setup
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(1000);
    Serial.print(".");
    attempts++;
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  }
  else {
    Serial.println("\nFailed to connect to WiFi.");
  }
  // 5. QRCode Setup
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(qrDataUrl);
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
      String payload = http.getString();
      Serial.print("Received QR data: ");
      Serial.println(payload);
      tft.fillScreen(TFT_WHITE);
      qrcode.init();
      qrcode.create(payload.c_str());
    } else {
      Serial.printf("Error getting QR data: %s\n", http.errorToString(httpCode).c_str());
      tft.fillScreen(TFT_RED);
      tft.setCursor(0, 0);
      tft.setTextColor(TFT_WHITE);
      tft.print("HTTP Error: ");
      tft.println(httpCode);
    }
    http.end();
  }
  else {
    tft.fillScreen(TFT_RED);
    tft.setCursor(0, 0);
    tft.setTextColor(TFT_WHITE);
    tft.println("No WiFi!");
    Serial.println("Cannot fetch QR data, WiFi not connected.");
  }
}

void loop() {
  if (ts.tirqTouched() && ts.touched()) {
    openDoorRequest();
    delay(100);
  }
}

void openDoorRequest() {
  HTTPClient http;
  http.begin(openDoorUrl);
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
    Serial.println("Puerta abierta :)");
  }
  else {
    Serial.println("Fracasamos y en general todo mal :(");
  }
  http.end();
}
