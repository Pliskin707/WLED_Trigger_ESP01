#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoOTA.h>

/** Wifi authentication **
 * 
 * this file needs to be created with the following content (and is obviously not included in version control):
 * 
 * #pragma once
 * 
 * const char* ssid = "<YourSSIDhere>";
 * const char* password = "<YourPasswordHere>";
 */
#include "../../../../../../wifiauth2.h"

#ifndef DEBUG_PRINT
#define dprintf(...) ;
#else
#define dprintf(fstr, ...) Serial.printf_P(PSTR(fstr), ##__VA_ARGS__);
#endif

#define LEDPIN  2
#define WLEDIP "192.168.0.24"
const char * deviceName = "WLED_Trigger";

static bool mDNS_init_ok = false;
WiFiClient client;
HTTPClient http;

void setup() {
  #ifndef DEBUG_PRINT
  pinMode(LEDPIN, OUTPUT);
  #else
  Serial.begin(115200);
  #endif

  // Wifit
  IPAddress local_IP(192, 168, 0, 40);
  IPAddress gateway(192, 168, 0, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress dns(1, 1, 1, 1);
  WiFi.hostname(deviceName);
  WiFi.mode(WIFI_STA);
  WiFi.config(local_IP, gateway, subnet, dns);
  WiFi.begin(ssid, password);
  wifi_set_sleep_type(NONE_SLEEP_T);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    #ifndef DEBUG_PRINT
    digitalWrite(LEDPIN, !digitalRead(LEDPIN));
    #else
    Serial.println(F("Connecting..."));
    #endif
  }

  // mDNS
  mDNS_init_ok = MDNS.begin(deviceName);
  // if (mDNS_init_ok)
  //   MDNS.addService("trigger", "tcp", 8080);

  // OTA
  ArduinoOTA.setHostname(deviceName);
  ArduinoOTA.onStart([]() 
  {
    wifi_set_sleep_type(NONE_SLEEP_T);
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    #ifndef DEBUG_PRINT
    digitalWrite(LEDPIN, !digitalRead(LEDPIN));
    #endif
  });
  ArduinoOTA.onError([](ota_error_t error) {
    ESP.reset();
  });
  ArduinoOTA.onEnd([]() {
    ESP.reset();
  });
  ArduinoOTA.begin();
}

void loop() {
  const uint32_t time = millis();
  static uint32_t next = 0;

  // Wifi status
  const bool connected = (WiFi.status() == WL_CONNECTED);
  #ifndef DEBUG_PRINT
  digitalWrite(LEDPIN, !connected);
  #endif

  // mDNS
  if (mDNS_init_ok)
    MDNS.update();

  // OTA
  ArduinoOTA.handle();

  // program logic
  if (time >= next)
  {
    next = time + 5000;
    dprintf("Systime: %lu ms\n", time);

    if (connected)
    {
      // HTTP
      http.begin(client, String("http://") + WLEDIP + "/json/state");
      const int result = http.GET();
      dprintf("GET result: %d\n", result);
      if (result == 200)  // == OK
      {
        dprintf("Response: \"%s\"\n", http.getString().c_str());
      }
      http.end();
    }
  }

  yield();
}