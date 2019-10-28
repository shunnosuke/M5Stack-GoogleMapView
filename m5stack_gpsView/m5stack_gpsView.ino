#include <M5Stack.h>
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include "arduino_secrets.h"
WiFiMulti wifiMulti;
const char* ssid     = SECRET_SSID;
const char* password = SECRET_PASS;
const char* api_key  = SECRET_APIK;
const char* language = "ja";
const char* region   = "JP";

uint8_t buff_pic[30000];
uint16_t buff_len = 0;

unsigned long previousMillis = 0;
const long interval = 5000;

String defaultLatitude        = "35.0364041";
String defaultLongitude       = "135.7613615";
unsigned char defaultZoomMode = 15;
String defaultMapType         = "roadmap";


void setup()
{
  //USE_SERIAL.begin(115200);
  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();
  M5.begin();
  for (uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }
  M5.Lcd.setBrightness(255);
  M5.update();
  header("Initial System", TFT_NAVY );
  M5.Lcd.setFreeFont(&FreeSansBold9pt7b);
  WiFi.disconnect(true);
  delay(1000);
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(ssid, password);
  lcd_serial_println("Connect AP");
  lcd_serial_println("");
  while (wifiMulti.run() != WL_CONNECTED) {
    delay(500);
    lcd_serial_print(".");
  }
  lcd_serial_println("");
  lcd_serial_println("WiFi connected");
  lcd_serial_print("IP address: ");
  lcd_serial_println(WiFi.localIP().toString());
  delay(5000);
  lcd_serial_println("Connect Server");

}

void loop()
{
  Get_GoogleMAP(defaultLatitude, defaultLongitude, defaultZoomMode, defaultMapType);
  delay(interval);
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
        WiFiClient * stream = http.getStreamPtr();
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
