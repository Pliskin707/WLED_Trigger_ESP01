#ifndef __WLEDCTRL_HPP__
#define __WLEDCTRL_HPP__

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "projutils/projutils.hpp"

namespace wledTrigger
{

class wledControl
{
    private:
        StaticJsonDocument<1024> _doc;  // should be enough, since the filter ignores most values anyway
        StaticJsonDocument<256> _filter;
        WiFiClient _client;
        String _url;
        bool _isOn = false;
        bool _wasOn = false;
        bool _isLive = false;
        int _brightness = -1;
        int _preset = -1;

        void _initFilter (StaticJsonDocument<256> &filter);

    public:
        wledControl(const IPAddress &WLEDIP);
        wledControl(const char * const WLEDIP);
        bool updateStatus (void);
        bool isOn (void) {return this->_isOn;};
        bool isLive (void) {return this->_isLive;};
        int getBrigthness (void) {return this->_brightness;};
        bool setPreset (const int presetNumber);    // also enables "live data override"
        bool resetLiveDataOverride (void);
};

}

#endif