#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include "ota/ota.hpp"
#include "wledctrl/wledctrl.hpp"
#include "projutils/projutils.hpp"
#include "config.hpp"

using namespace wledTrigger;

/** Wifi authentication **
 * 
 * this file needs to be created with the following content (and is obviously not included in version control):
 * 
 * #pragma once
 * 
 * const char* ssid2 = "<YourSSIDhere>";
 * const char* password = "<YourPasswordHere>";
 */
#include "../../../../../../wifiauth2.h"

static bool mDNS_init_ok = false;
WiFiClient client;
HTTPClient http;

void setup() {
  #ifndef DEBUG_PRINT
  pinMode(LEDPIN, OUTPUT);
  pinMode(BTNGATE, INPUT_PULLUP);
  pinMode(BTNDOOR, INPUT_PULLUP);
  #else
  Serial.begin(115200);
  #endif

  // Wifit
  IPAddress local_IP(192, 168, 0, 40);
  IPAddress gateway(192, 168, 0, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress dns(1, 1, 1, 1);
  WiFi.hostname(DEVICENAME);
  WiFi.mode(WIFI_STA);
  // WiFi.config(local_IP, gateway, subnet, dns);
  WiFi.begin(ssid2, password);
  wifi_set_sleep_type(NONE_SLEEP_T);

  wl_status_t wstat;
  while (true)
  {
    delay(500);
    wstat = WiFi.status();
    if (wstat == WL_CONNECTED)
      break;

    #ifndef DEBUG_PRINT
    digitalWrite(LEDPIN, !digitalRead(LEDPIN));
    #endif
    dprintf("Connecting (%d) ...\n", wstat);
  };

  // mDNS
  mDNS_init_ok = MDNS.begin(DEVICENAME);
  // if (mDNS_init_ok)
  //   MDNS.addService("trigger", "tcp", 8080);

  // OTA
  ota::begin(DEVICENAME);
}

void loop() {
  const uint32_t time = millis();
  static uint32_t next = 0;
  static wledControl wled(WLEDIP);

  // Wifi status
  const bool connected = WiFi.isConnected();
  #ifndef DEBUG_PRINT
  digitalWrite(LEDPIN, !connected);
  #endif

  // mDNS
  if (mDNS_init_ok)
    MDNS.update();

  // OTA
  ota::handle();

  // program logic
  if (time >= next)
  {
    next = time + 10000;
    dprintf("Systime: %lu ms; WLAN: %sconnected (as %s)\n", time, (connected ? "":"dis"), WiFi.localIP().toString().c_str());

    if (connected)
    {
      if (wled.updateStatus())
      {
        dprintf("WLED state: %s and %slive with %.2f%% brightness\n", (wled.isOn() ? "ON":"OFF"), (wled.isLive() ? "":"not "), ((float) wled.getBrigthness()) / 2.55f);
      }

      #ifdef DEBUG_PRINT
      // auto toggle since the usb-programmer has no buttons attached
      static bool presetApplied = false;
      if (presetApplied)
        wled.resetLiveDataOverride();
      else
        wled.setPreset(3);

      presetApplied ^= true;
      #endif
    }
    else
      WiFi.reconnect();
  }

  #ifndef DEBUG_PRINT
  if (connected)
  {
    // read the button pins
    static bool prevBtnDoor = HIGH, prevBtnGate = HIGH, isSet = false;
    const bool btnDoor = digitalRead(BTNDOOR), btnGate = digitalRead(BTNGATE);
    static uint32_t activationTime = 0;
    
    if (!isSet)
    {
      // activate?
      if (prevBtnDoor && !btnDoor)
      {
        isSet = wled.setPreset(4);
      }
      else if (prevBtnGate && !btnGate)
      {
        isSet = wled.setPreset(5);
      }

      if (isSet)
        activationTime = time;
    }
    else
    {
      // deactivate
      if (time > (activationTime + 3000))
        isSet = !wled.resetLiveDataOverride();
    }

    prevBtnDoor = btnDoor;
    prevBtnGate = btnGate;
  }
  #endif

  yield();
}