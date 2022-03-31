#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
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

#define LEDPIN  2
const char * deviceName = "WLED_Trigger";

static bool mDNS_init_ok = false;

void setup() {
  pinMode(LEDPIN, OUTPUT);

  // Wifi
  IPAddress local_IP(192, 168, 0, 40);
  IPAddress gateway(192, 168, 0, 1);
  IPAddress subnet(255, 255, 255, 0);
  WiFi.config(local_IP, gateway, subnet);
  wifi_set_sleep_type(NONE_SLEEP_T);
  WiFi.hostname(deviceName);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    digitalWrite(LEDPIN, !digitalRead(LEDPIN));
  }

  // mDNS
  mDNS_init_ok = MDNS.begin(deviceName);
  // if (mDNS_init_ok)
  //   MDNS.addService("display", "tcp", TCP_PORT);

  // OTA
  ArduinoOTA.setHostname(deviceName);
  ArduinoOTA.onStart([]() 
  {
    wifi_set_sleep_type(NONE_SLEEP_T);
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    digitalWrite(LEDPIN, !digitalRead(LEDPIN));
  });
  ArduinoOTA.onEnd([]() {
    ESP.reset();
  });
  ArduinoOTA.begin();
}

void loop() {
  // Wifi status
  const bool connected = (WiFi.status() == WL_CONNECTED);
  digitalWrite(LEDPIN, !connected);

  // mDNS
  if (mDNS_init_ok)
    MDNS.update();

  // OTA
  ArduinoOTA.handle();

  yield();
}