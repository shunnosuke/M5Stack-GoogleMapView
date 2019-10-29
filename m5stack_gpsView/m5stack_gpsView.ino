#include <M5Stack.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <TinyGPS++.h>
// #include <SoftwareSerial.h>
#include "arduino_secrets.h"

static const char* ssid     = SECRET_SSID;
static const char* password = SECRET_PASS;
static const char* api_key  = SECRET_APIK;
static const char* language = "ja";
static const char* region   = "JP";
WiFiMulti wifiMulti;

static const uint32_t GPSBaud = 9600;
static const int GPS_TX       = 0;
static const int GPS_RX       = 15;
static const int GPS_PPS      = 34;
TinyGPSPlus gps; // The TinyGPS++ object
HardwareSerial ss(2); // The defualt is 2
// SoftwareSerial ss; // EspSoftwareSerial

uint8_t buff_pic[30000];
uint16_t buff_len = 0;
const long interval = 5000;

String defaultLatitude    = "35.0364041";
String defaultLongitude   = "135.7613615";
unsigned char defaultZoom = 15;
String defaultMapType     = "roadmap";


void setup()
{
  //USE_SERIAL.begin(115200);
  USE_SERIAL.println();
  M5.begin();
  ss.begin(GPSBaud, SERIAL_8N1, GPS_TX, GPS_RX);
  for (uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }
  M5.Lcd.setBrightness(255);
  M5.update();
  header("Initializing System", TFT_NAVY );
  M5.Lcd.setFreeFont(&FreeSansBold9pt7b);
  lcd_serial_println("");
  lcd_serial_println("");
  lcd_serial_println("");
  lcd_serial_print("TinyGPS++ library v. ");
  lcd_serial_println(TinyGPSPlus::libraryVersion());
  WiFi.disconnect(true);
  delay(1000);
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(ssid, password);
  lcd_serial_println("Connect AP");
  while (wifiMulti.run() != WL_CONNECTED) {
    delay(500);
    lcd_serial_print(".");
  }
  lcd_serial_println("WiFi connected");
  lcd_serial_print("IP address: ");
  lcd_serial_println(WiFi.localIP().toString());
  delay(5000);
  lcd_serial_println("Connect Server");

}

void loop()
{
  String latitude = String(gps.location.lat(), 7);
  String longitude = String(gps.location.lng(), 7);
  USE_SERIAL.println("[GPS] " + latitude + ", " + longitude);
  Get_GoogleMAP(latitude, longitude, defaultZoom, defaultMapType);
  smartDelay(interval);
}
String GennerateGet(String latitude, String longitude, unsigned char zoom, String maptype)
{
  String data = "http://maps.googleapis.com/maps/api/staticmap?";
  data += "center=" + latitude + "," + longitude;
  data += "&format=jpg-baseline";
  data += "&zoom=" + String(zoom);
  data += "&size=320x240";
  data += "&maptype=" + maptype;
  data += "&markers=color:red%7Clabel:S%7C" + latitude + "," + longitude;
  data += "&language=" + String(language);
  data += "&region=" + String(region);
  data += "&key=" + String(api_key);
  return (data);
}
void Get_GoogleMAP(String latitude, String longitude, unsigned char ZoomMode, String MapType)
{

  if (true /* wifiMulti.run() == WL_CONNECTED */) {
    String url = GennerateGet(latitude, longitude, ZoomMode, MapType);
    HTTPClient http;
    USE_SERIAL.print("[HTTP] begin...\n");
    USE_SERIAL.print("[HTTP] GET... ");
    USE_SERIAL.println(url);
    http.begin(url);
    int httpCode = http.GET();
    if (httpCode > 0) {
      USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK) {
        int len = http.getSize();
        uint8_t buff[128] = { 0 };
        WiFiClient* stream = http.getStreamPtr();
        buff_len = 0;
        while (http.connected() && (len > 0 || len == -1)) {
          size_t size = stream->available();

          if (size) {
            int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
            for (int i = 0; i < c; i++) {
              buff_pic[buff_len] = buff[i];
              buff_len++;
            }
            if (len > 0) {
              len -= c;
            }
          }
          delay(1);
        }
        M5.Lcd.drawJpg(buff_pic, buff_len);
        USE_SERIAL.print("[HTTP] connection closed or file end.\n");
        USE_SERIAL.println();
      }
    } else {
      USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
  }
}
static void smartDelay(unsigned long ms)
{
  USE_SERIAL.print("[SERIAL] {");
  unsigned long start = millis();
  do {
    while (ss.available())
      // USE_SERIAL.print(ss.read());
      gps.encode(ss.read());
  } while (millis() - start < ms);
  USE_SERIAL.println("}");
}



void header(const char *string, uint16_t color)
{
  M5.Lcd.fillScreen(color);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(TFT_MAGENTA, TFT_BLUE);
  M5.Lcd.fillRect(0, 0, 320, 30, TFT_BLUE);
  M5.Lcd.setTextDatum(TC_DATUM);
  M5.Lcd.drawString(string, 160, 2, 4); // Font 4 for fast drawing with background
}
void lcd_serial_print(String str)
{
  M5.Lcd.print(" ");
  M5.Lcd.print(str);
  USE_SERIAL.print(str);
}
void lcd_serial_println(String str)
{
  M5.Lcd.print(" ");
  M5.Lcd.println(str);
  USE_SERIAL.println(str);
}
