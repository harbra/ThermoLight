/* ThermoLight.h

Purpose
=======
This is the main ThermoLight library encapsulating all functionality.

License
=======

MIT License

Copyright (c) 2019 Harald Braun

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef ThermoLight_h
#define ThermoLight_h

#include <WebController.h>
#include <AsyncHTTPRequest.hpp>
#include <DoubleBarController.h>
#include <SensorReader.h>
#include <ColorHelper.hpp>

#include <FS.h>
#include <Logger.h>

#define ARDUINOJSON_USE_LONG_LONG 1
//#include <AsyncJson.h>
#include <ArduinoJson.h>

#include "FilesToJSON.hpp"

#include <stdlib.h>
#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <WProgram.h>
#include <wiring.h>
#endif

typedef std::function<void(double value)> ValueUpdateCallback;

class ThermoLight
{
public:

  ThermoLight();

  // should be called in Arduino setup function
  void setup();

  // should be called in Arduino loop function
  void handle();

protected:


private:

  typedef enum {
    Base,
    Sensor,
    HTTP,
    MQTT,
    Test
  } ValueProviderType;

  class ValueProviderBase
  {
  public:
    virtual double getValue() { return 0.0; };
    virtual void handle() {};

    ValueProviderType getType() { return m_providertype; };

    virtual ~ValueProviderBase() {};

  protected:
    //ValueProviderBase() {};
    ValueProviderBase(ValueProviderType providerType) : m_providertype(providerType) {};

    std::vector<ValueUpdateCallback> m_updateCallback;

    unsigned long m_updateInterval=0;
    unsigned long m_lastMillis=0;

    ValueProviderType m_providertype=ValueProviderType::Base;
  };

  class ValueProviderSensor : public ValueProviderBase
  {
  public:
    typedef enum {
      Temperature,
      Humidity,
      eCO2,
      TVOC
    } SensorSource;

    ValueProviderSensor(SensorReader &sensors, SensorSource source, ValueUpdateCallback UpdateCallback, unsigned long UpdateInterval) : ValueProviderBase(ValueProviderType::Sensor), m_sensors(sensors), m_source(source) {
      m_updateCallback.push_back(UpdateCallback);
      m_updateInterval=UpdateInterval;
    };

    virtual void handle() {
      if(millis()-m_lastMillis>m_updateInterval) {
        m_updateCallback[0](getValue());

        m_lastMillis=millis();
      }
    }

    virtual double getValue() {
      switch(m_source) {
        case SensorSource::Temperature:
          return m_sensors.getTemperature();
          break;
        case SensorSource::Humidity:
          return m_sensors.getHumidity();
          break;
        case SensorSource::eCO2:
          return m_sensors.geteCO2();
          break;
        case SensorSource::TVOC:
          return m_sensors.getTVOC();
          break;
        default:
          return 0.0;
          break;
      }
    };

  private:
    SensorReader& m_sensors;
    SensorSource m_source;
  };

  class ValueProviderMQTT : public ValueProviderBase
  {
  public:
    ValueProviderMQTT(WebController &webcontroller, String topic, double multiplicator, double summand, ValueUpdateCallback UpdateCallback) : ValueProviderBase(ValueProviderType::MQTT), m_webcontroller(webcontroller), m_topic(topic), m_multiplicator(multiplicator), m_summand(summand) {
      m_updateCallback.push_back(UpdateCallback);

      m_callbackvectorid=m_webcontroller.subscribeMQTT(m_topic, std::bind(&ValueProviderMQTT::mqttCallback, this, std::placeholders::_1, std::placeholders::_2));
    };

    virtual ~ValueProviderMQTT() {
      m_webcontroller.unsubscribeMQTT(m_topic,m_callbackvectorid);
    };

  private:
    WebController& m_webcontroller;
    unsigned int m_callbackvectorid;
    String m_topic;
    double m_multiplicator;
    double m_summand;

    void mqttCallback(String topic, String content) {
      m_updateCallback[0](m_multiplicator*content.toFloat()+m_summand);
    };
  };

  class ValueProviderHTTP : public ValueProviderBase
  {
  public:
    ValueProviderHTTP(String url, int port, std::vector<String> jsonkeys, double multiplicator, double summand, ValueUpdateCallback UpdateCallback, unsigned long UpdateInterval) : ValueProviderBase(ValueProviderType::HTTP), m_httpRequest(url, port), m_buffer(nullptr) {
      m_updateInterval=UpdateInterval;

      m_jsonkeys.push_back(jsonkeys);
      m_updateCallback.push_back(UpdateCallback);
      m_multiplicators.push_back(multiplicator);
      m_summands.push_back(summand);
    };

    void addJSONkeys(std::vector<String> jsonkeys, double multiplicator, double summand, ValueUpdateCallback UpdateCallback) {
      m_jsonkeys.push_back(jsonkeys);
      m_updateCallback.push_back(UpdateCallback);
      m_multiplicators.push_back(multiplicator);
      m_summands.push_back(summand);
    };

    String getURL() {
      return m_httpRequest.getUrl();
    }

    int getPort() {
      return m_httpRequest.getPort();
    }

    virtual void handle() {
      if(m_buffer!=nullptr) {
        myLogger.addToLog("Processing HTTP response");

        DynamicJsonBuffer jsonBuffer;
        JsonObject& root = jsonBuffer.parseObject(m_buffer);
        if(root!=JsonObject::invalid()) {
          for(unsigned int keyid=0; keyid<m_jsonkeys.size(); keyid++) {
            unsigned int idx=0;
            JsonObject* objiterator=&root;
            while(idx<m_jsonkeys[keyid].size()-1) {
              if(objiterator->containsKey(m_jsonkeys[keyid][idx])) {
                JsonObject& newIterator=(*objiterator)[m_jsonkeys[keyid][idx]].as<JsonObject>();
                objiterator=&newIterator;
                if(*objiterator==JsonObject::invalid()) break;
                idx++;
              }
            }

            if(*objiterator!=JsonObject::invalid()) {
              if(objiterator->containsKey(m_jsonkeys[keyid][m_jsonkeys[keyid].size()-1])) {
                m_updateCallback[keyid](m_multiplicators[keyid]*((*objiterator)[m_jsonkeys[keyid][m_jsonkeys[keyid].size()-1]].as<float>())+m_summands[keyid]);
              }
            }
          }
        }

        m_buffer=nullptr;
        m_httpRequest.clearBuffer();
      }

      if(millis()-m_lastMillis>m_updateInterval) {
        myLogger.addToLog("Sending HTTP request");
        m_httpRequest.send(std::bind(&ValueProviderHTTP::processResponse, this, std::placeholders::_1, std::placeholders::_2), false);

        m_lastMillis=millis();
      }
    }

  private:
    AsyncHTTPRequest m_httpRequest;
    std::vector<std::vector<String>> m_jsonkeys;
    std::vector<double> m_multiplicators;
    std::vector<double> m_summands;
    char* m_buffer;

    void processResponse(int status, char* body) {
      if(status==200) {
        m_buffer=body;
      }
    }
  };

  class ValueProviderTest : public ValueProviderBase
  {
  public:
    ValueProviderTest(double lowerBound, double upperBound, int steps, ValueUpdateCallback UpdateCallback, unsigned long UpdateInterval) : ValueProviderBase(ValueProviderType::Test), m_lowerBound(lowerBound), m_upperBound(upperBound), m_currentValue(lowerBound), m_delta((m_upperBound-m_lowerBound)/double(steps)) {
      m_updateCallback.push_back(UpdateCallback);
      m_updateInterval=UpdateInterval;
    };

    virtual void handle() {
      if(millis()-m_lastMillis>m_updateInterval) {
        m_updateCallback[0](m_currentValue);

        m_currentValue+=m_delta;
        if(m_currentValue>m_upperBound) {
          m_currentValue=m_upperBound;
          m_delta*=-1;
        } else if(m_currentValue<m_lowerBound) {
          m_currentValue=m_lowerBound;
          m_delta*=-1;
        }

        m_lastMillis=millis();
      }
    }

    double getLowerBound() { return m_lowerBound; };
    double getUpperBound() { return m_upperBound; };
    double getDelta() { return m_delta; };

  private:
    double m_lowerBound;
    double m_upperBound;
    double m_currentValue;
    double m_delta;
  };

  WebController controller;
  DoubleBarController barController;
  SensorReader sensors;

  std::array<std::array<std::unique_ptr<ValueProviderBase>,2>,2> m_valueProvider;
  std::unique_ptr<ValueProviderBase> m_brightnessValueProvider;

  typedef enum {
    Brightness,
    SourcesBrightness,
    Sources,
    Left,
    Right,
    SettingsType_MAX=Right
  } SettingsType;

  std::array<const char*, SettingsType::SettingsType_MAX+1> m_settingsfilename;
  bool m_reloadsettings;
  bool m_writenewsettings;

  void handleGetConfig(AsyncWebServerRequest * request, const char* jsonfilename, String subsetting);
  void handleGetConfigAccumulated(AsyncWebServerRequest * request);

  void handleSetConfig(AsyncWebServerRequest * request, bool isLeft, bool isPrimary);
  void handleSetConfigAccumulated(AsyncWebServerRequest * request);
  void handleSetConfigGeneric(JsonObject &settingstoedit, JsonArray &newsettings, bool isPrimary);

  void handleSetBrightness(AsyncWebServerRequest * request);

  void handleGetColorScale(AsyncWebServerRequest * request);
  String getColorString(ColorHelper::RGB color1, ColorHelper::RGB color2, int upperlimit);

  void handleGetSensorValues(AsyncWebServerRequest * request);

  void sendNewSensorValue(String mqtttopic, String sensoreventstring, double value);

  bool validateSettings(JsonArray &newsettings, JsonArray & sources);
  bool validateBrightness(JsonArray &newsettings, JsonArray &brightnesssources);

  bool ValueProviderHTTPexists(String url, int port, int &ret_side, int &ret_primsec);

  void mqttCallback(String, std::unique_ptr<char []>);
  void setBrightness(double value, double lowerbound, double upperbound, bool zeroiflower);

  void writeNewSettings();
  bool loadSettings();

  void configureDisplay(DoubleBarController::BarSide side, JsonArray& primaryvalues, JsonArray& primarycolors);
  void configureDisplay(DoubleBarController::BarSide side, JsonArray& primaryvalues, JsonArray& secondaryvalues, JsonArray& secondarycolors);

};

#endif
