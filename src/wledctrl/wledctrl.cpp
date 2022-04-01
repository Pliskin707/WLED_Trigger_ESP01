#include "wledctrl.hpp"

namespace wledTrigger
{
enum LorOverrideType {off, untilLiveEnds, untilReboot};

wledControl::wledControl (const IPAddress &WLEDIP)
{
    this->_url = "http://" + WLEDIP.toString() + "/json";
    this->_initFilter(this->_filter);
}

wledControl::wledControl(const char * const WLEDIP)
{
    this->_url = "http://" + String(WLEDIP) + "/json";
    this->_initFilter(this->_filter);
}

void wledControl::_initFilter (StaticJsonDocument<256> &filter)
{
    filter.clear();

    // from "state"
    filter["on"] = true;
    filter["bri"] = true;
    filter["ps"] = true;
    filter["lor"] = true;

    // from "info"
    filter["live"] = true;
}

bool wledControl::updateStatus (void)
{
    bool getStateSuccess = false, getInfoSuccess = false;
    int result;

    HTTPClient http;
    JsonVariant jVal;

    // temporary values (get saved if all requests are successful)
    bool isOn = false, isLive = false;
    int brightness = -1, liveDataOverride = -1, preset = -1;

    // client settings
    http.setTimeout(3000);
    http.useHTTP10(true);   // this is required, so "deserializeJson()" can read the stream directly

    // get "state" data
    http.begin(this->_client, this->_url + "/state");
    result = http.GET();
    dprintf("GET state result: %d\n", result);
    if (result == 200)  // == OK
    {
        DeserializationError error = deserializeJson(this->_doc, http.getStream(), DeserializationOption::Filter(this->_filter));
        
        if (error)
        {
            dprintf("Deserialization failed: %s\n", error.c_str());
        }
        else
        {
            do
            {
                jVal = this->_doc["on"];
                if (jVal.is<bool>())
                    isOn = jVal.as<bool>();
                else
                    break;

                jVal = this->_doc["bri"];
                if (jVal.is<int>())
                    brightness = jVal.as<int>();
                else
                    break;

                jVal = this->_doc["lor"];
                if (jVal.is<int>())
                    liveDataOverride = jVal.as<int>();
                else
                    break;

                jVal = this->_doc["ps"];
                if (jVal.is<int>())
                    preset = jVal.as<int>();
                else
                    break;

                getStateSuccess = true;
            } while (false);
        }
    }

    // get "info" data
    http.begin(this->_client, this->_url + "/info");
    result = http.GET();
    dprintf("GET info result: %d\n", result);
    if (result == 200)  // == OK
    {
        DeserializationError error = deserializeJson(this->_doc, http.getStream(), DeserializationOption::Filter(this->_filter));
        
        if (error)
        {
            dprintf("Deserialization failed: %s\n", error.c_str());
        }
        else
        {
            do
            {
                jVal = this->_doc["live"];
                if (jVal.is<bool>())
                    isLive = jVal.as<bool>();
                else
                    break;

                getInfoSuccess = true;
            } while (false);
        }
    }

    http.end();

    // save values
    if (getStateSuccess && getInfoSuccess)
    {
        this->_isOn = isOn;
        this->_isLive = isLive;
        this->_brightness = brightness;

        // save the previously active preset so it can be restored later
        if (liveDataOverride == LorOverrideType::off)
        {
            dprintf("Saving actual settings: Preset = %d, Power = %s\n", preset, (isOn ? "ON":"OFF"));
            this->_preset = preset;
            this->_wasOn = isOn;
        }

        return true;
    }

    return false;
}

bool wledControl::setPreset (const int presetNumber)
{
    auto &doc = this->_doc;
    
    
    doc.clear();
    doc["on"] = true;   // not required? always turns on
    doc["tt"] = 0;  // transition time (for this call)
    doc["ps"] = presetNumber;
    doc["lor"] = LorOverrideType::untilLiveEnds;    // live data override

    String json;
    serializeJson(doc, json);

    HTTPClient http;
    http.begin(this->_client, this->_url);
    const int result = http.POST(json);

    dprintf("POST response (%d): %s\n", result, http.getString().c_str());

    return result == 200;
}

bool wledControl::resetLiveDataOverride (void)
{
    auto &doc = this->_doc;
    
    doc.clear();
    doc["lor"] = LorOverrideType::off;  // live data override

    if ((this->_preset >= 0) && this->_wasOn)
        doc["ps"] = this->_preset;          // restore 
    
    doc["on"] = this->_wasOn;

    String json;
    serializeJson(doc, json);

    HTTPClient http;
    http.begin(this->_client, this->_url);
    const int result = http.POST(json);

    dprintf("LOR reset response (%d): %s\n", result, http.getString().c_str());

    return result == 200;
}

}