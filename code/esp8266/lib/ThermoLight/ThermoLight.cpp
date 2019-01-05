/*
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

#include "ThermoLight.h"

ThermoLight::ThermoLight() : barController(14,19,7), m_settingsfilename({"/config/brightness.json","/config/sourcesbrightness.json","/config/sources.json","/config/left.json","/config/right.json"}), m_reloadsettings(false), m_writenewsettings(false)
{
}

void ThermoLight::setup()
{
    myLogger.addToLog(F("Initializing ThermoLight..."));
    SPIFFS.begin();

    controller.setup();

    controller.addSettingsIncludePath("/config/");

    controller.addServerHandler("/api/brightness",HTTP_GET,std::bind(&ThermoLight::handleGetConfig, this, std::placeholders::_1, m_settingsfilename[SettingsType::Brightness], ""), false, false);
    controller.addServerHandler("/api/brightness", HTTP_PUT, std::bind(&ThermoLight::handleSetBrightness, this, std::placeholders::_1), false, true);

    controller.addServerHandler("/api/sources",HTTP_GET,std::bind(&ThermoLight::handleGetConfig, this, std::placeholders::_1, m_settingsfilename[SettingsType::Sources], ""), false, false);

    controller.addServerHandler("/api/colors", HTTP_GET, std::bind(&ThermoLight::handleGetColorScale, this, std::placeholders::_1), false, false);
    controller.addServerHandler("/api/sensorvalues", HTTP_GET, std::bind(&ThermoLight::handleGetSensorValues, this, std::placeholders::_1), false, false);

    controller.addServerHandler("/api/displaysettings", HTTP_GET, std::bind(&ThermoLight::handleGetConfigAccumulated, this, std::placeholders::_1), false, false);
    controller.addServerHandler("/api/displaysettings", HTTP_PUT, std::bind(&ThermoLight::handleSetConfigAccumulated, this, std::placeholders::_1), false, true);

    controller.addServerHandler("/api/left/primary",HTTP_GET,std::bind(&ThermoLight::handleGetConfig, this, std::placeholders::_1, m_settingsfilename[SettingsType::Left], "primary"), false, false);
    controller.addServerHandler("/api/left/secondary",HTTP_GET,std::bind(&ThermoLight::handleGetConfig, this, std::placeholders::_1, m_settingsfilename[SettingsType::Left], "secondary"), false, false);
    controller.addServerHandler("/api/left",HTTP_GET,std::bind(&ThermoLight::handleGetConfig, this, std::placeholders::_1, m_settingsfilename[SettingsType::Left], ""), false, false);

    controller.addServerHandler("/api/right/primary",HTTP_GET,std::bind(&ThermoLight::handleGetConfig, this, std::placeholders::_1, m_settingsfilename[SettingsType::Right], "primary"), false, false);
    controller.addServerHandler("/api/right/secondary",HTTP_GET,std::bind(&ThermoLight::handleGetConfig, this, std::placeholders::_1, m_settingsfilename[SettingsType::Right], "secondary"), false, false);
    controller.addServerHandler("/api/right",HTTP_GET,std::bind(&ThermoLight::handleGetConfig, this, std::placeholders::_1, m_settingsfilename[SettingsType::Right], ""), false, false);

    controller.addServerHandler("/api/left/primary", HTTP_PUT, std::bind(&ThermoLight::handleSetConfig, this, std::placeholders::_1, true, true), false, true);
    controller.addServerHandler("/api/left/secondary", HTTP_PUT, std::bind(&ThermoLight::handleSetConfig, this, std::placeholders::_1, true, false), false, true);
    controller.addServerHandler("/api/right/primary", HTTP_PUT, std::bind(&ThermoLight::handleSetConfig, this, std::placeholders::_1, false, true), false, true);
    controller.addServerHandler("/api/right/secondary", HTTP_PUT, std::bind(&ThermoLight::handleSetConfig, this, std::placeholders::_1, false, false), false, true);

    sensors.setup(0x44,0x5B);
    sensors.attachHandlers(std::bind(&ThermoLight::sendNewSensorValue, this, String("/telemetry/erlangen/livingroom/thermolight/temperature"), String("temperature"), std::placeholders::_1),
                           std::bind(&ThermoLight::sendNewSensorValue, this, String("/telemetry/erlangen/livingroom/thermolight/humidity"), String("humidity"), std::placeholders::_1),
                           std::bind(&ThermoLight::sendNewSensorValue, this, String("/telemetry/erlangen/livingroom/thermolight/eCO2"), String("eco2"), std::placeholders::_1),
                           std::bind(&ThermoLight::sendNewSensorValue, this, String("/telemetry/erlangen/livingroom/thermolight/TVOC"), String("tvoc"), std::placeholders::_1));

    barController.setup();
    barController.setBrightness(50);

    loadSettings();
}

void ThermoLight::handle()
{
    controller.loop();
    barController.handle();
    sensors.handle();
    for(unsigned int i=0; i<m_valueProvider.size(); i++) {
      for(unsigned int j=0; j<m_valueProvider[i].size(); j++) {
        if(m_valueProvider[i][j]!=nullptr) {
          m_valueProvider[i][j]->handle();
        }
      }
    }
    if(m_brightnessValueProvider!=nullptr) {
      m_brightnessValueProvider->handle();
    }

    if(m_writenewsettings) {
      m_writenewsettings=false;
      writeNewSettings();
    }

    if(m_reloadsettings) {
      m_reloadsettings=false;
      loadSettings();
    }
}

void ThermoLight::configureDisplay(DoubleBarController::BarSide side, JsonArray& primaryvalues, JsonArray& primarycolors)
{
  barController.setPrimaryScale(side, primaryvalues[0].as<double>(), primaryvalues[primaryvalues.size()-1].as<double>());
  barController.setSecondaryScalePrimaryOnly(side, primaryvalues, primarycolors);
  barController.setMode(side,DoubleBarController::DisplayMode::PrimaryOnly);
  barController.displayValue(side, primaryvalues[0].as<double>()+(primaryvalues[primaryvalues.size()-1].as<double>()-primaryvalues[0].as<double>())/2.0);
}

void ThermoLight::configureDisplay(DoubleBarController::BarSide side, JsonArray& primaryvalues, JsonArray& secondaryvalues, JsonArray& secondarycolors)
{
  barController.setPrimaryScale(side, primaryvalues[0].as<double>(), primaryvalues[primaryvalues.size()-1].as<double>());
  barController.setSecondaryScaleSecondarySingleColor(side, secondaryvalues, secondarycolors);
  barController.setMode(side,DoubleBarController::DisplayMode::SecondarySingleColor);
  barController.displayValue(side, primaryvalues[0].as<double>()+(primaryvalues[primaryvalues.size()-1].as<double>()-primaryvalues[0].as<double>())/2.0);
  barController.displaySecondaryValue(side, secondaryvalues[0].as<double>()+(secondaryvalues[secondaryvalues.size()-1].as<double>()-secondaryvalues[0].as<double>())/2.0);
}

bool ThermoLight::loadSettings()
{
  myLogger.addToLog(F("Loading ThermoLight Settings..."));

  for(int side=0; side<2; side++) {
    for(int j=0; j<2; j++) {
      m_valueProvider[side][j]=nullptr;
    }
  }

  for(int allsettings=0; allsettings<=SettingsType::SettingsType_MAX; allsettings++)
  {
    File settingsFile=SPIFFS.open(m_settingsfilename[allsettings],"r");

    yield();

    if(allsettings==SettingsType::Brightness) {
      DynamicJsonBuffer jsonBufferSettings;
      JsonArray& brightnesssettings = jsonBufferSettings.parseArray(settingsFile);

      if(brightnesssettings==JsonArray::invalid()) {
        myLogger.addToLog(F("Couldn't parse settingsfile: "),m_settingsfilename[allsettings]);

        settingsFile.close();
        continue;
      }

      String brightnesstype=brightnesssettings[0].as<String>();

      if(brightnesstype.equalsIgnoreCase("fixed")) {
        m_brightnessValueProvider=nullptr;
        barController.setBrightness(brightnesssettings[1].as<unsigned int>());
      } else if(brightnesstype.equalsIgnoreCase("MQTT")) {
        double multiplicator=(brightnesssettings[3].as<double>()-brightnesssettings[2].as<double>())/(brightnesssettings[5].as<double>()-brightnesssettings[4].as<double>());
        if((brightnesssettings[5].as<double>()-brightnesssettings[4].as<double>())==0) multiplicator=0.;
        double summand=-multiplicator+brightnesssettings[2].as<double>();
        m_brightnessValueProvider=std::unique_ptr<ValueProviderMQTT>(new ValueProviderMQTT(controller,
                                                                                           brightnesssettings[1].as<String>(),
                                                                                           multiplicator,
                                                                                           summand,
                                                                                           std::bind(&ThermoLight::setBrightness, this, std::placeholders::_1, brightnesssettings[2].as<double>(), brightnesssettings[3].as<double>(), brightnesssettings[6].as<bool>())));
      }
    } else if(allsettings==SettingsType::Left || allsettings==SettingsType::Right) {
      DoubleBarController::BarSide side=allsettings==SettingsType::Left ? DoubleBarController::BarSide::Left : DoubleBarController::BarSide::Right;

      DynamicJsonBuffer jsonBufferSettings;
      JsonObject& setting = jsonBufferSettings.parseObject(settingsFile);

      if(setting==JsonObject::invalid()) {
        myLogger.addToLog(F("Couldn't parse settingsfile: "),m_settingsfilename[allsettings]);

        settingsFile.close();
        continue;
      }

      JsonArray& primarysetting=setting["primary"].as<JsonArray>();
      JsonArray& secondarysetting=setting["secondary"].as<JsonArray>();

      String primarysettingstring=primarysetting[0].as<String>();
      if(primarysettingstring.equalsIgnoreCase("none")) {
        m_valueProvider[side][0]=nullptr;
        m_valueProvider[side][1]=nullptr;

        barController.setMode(side,DoubleBarController::DisplayMode::None);
        barController.clear(side);
      } else {
        if(primarysettingstring.equalsIgnoreCase("temperature") || primarysettingstring.equalsIgnoreCase("humidity") || primarysettingstring.equalsIgnoreCase("eCO2") || primarysettingstring.equalsIgnoreCase("TVOC")) {
          ValueProviderSensor::SensorSource ssource;
          if(primarysettingstring.equalsIgnoreCase("temperature")) {
            ssource=ValueProviderSensor::SensorSource::Temperature;
          } else if (primarysettingstring.equalsIgnoreCase("humidity")) {
            ssource=ValueProviderSensor::SensorSource::Humidity;
          } else if (primarysettingstring.equalsIgnoreCase("eCO2")) {
            ssource=ValueProviderSensor::SensorSource::eCO2;
          } else if (primarysettingstring.equalsIgnoreCase("TVOC")) {
            ssource=ValueProviderSensor::SensorSource::TVOC;
          }

          m_valueProvider[side][0]=std::unique_ptr<ValueProviderSensor>(new ValueProviderSensor(sensors,
                                                                                                ssource,
                                                                                                std::bind(&DoubleBarController::displayValue, &this->barController, side, std::placeholders::_1),
                                                                                                primarysetting[3].as<int>()));
        } else if(primarysettingstring.equalsIgnoreCase("MQTT")) {
          if(primarysetting[4].is<JsonArray>() && primarysetting[4].as<JsonArray>()[0].as<String>().equalsIgnoreCase("MQTT")) {
            JsonArray& mqttsettingsarray=primarysetting[4].as<JsonArray>();

            m_valueProvider[side][0]=std::unique_ptr<ValueProviderMQTT>(new ValueProviderMQTT(controller,
                                                                                              mqttsettingsarray[1].as<String>(),
                                                                                              mqttsettingsarray[2].as<double>(),
                                                                                              mqttsettingsarray[3].as<double>(),
                                                                                              std::bind(&DoubleBarController::displayValue, &this->barController, side, std::placeholders::_1)));
          }
        } else if(primarysettingstring.equalsIgnoreCase("HTTP")) {
          if(primarysetting[4].is<JsonArray>() && primarysetting[4].as<JsonArray>()[0].as<String>().equalsIgnoreCase("HTTP")) {
            JsonArray& mqttsettingsarray=primarysetting[4].as<JsonArray>();

            std::vector<String> jsonkeys;
            if(mqttsettingsarray[3].is<JsonArray>()) {
              JsonArray& keyarray=mqttsettingsarray[3].as<JsonArray>();
              for(unsigned int i=0; i<keyarray.size(); i++) {
                jsonkeys.push_back(keyarray[i].as<String>());
              }
            } else if(mqttsettingsarray[3].is<char*>()) {
              jsonkeys.push_back(mqttsettingsarray[3].as<String>());
            }
            int exists_side=0; int exists_primsec=0;
            bool result=ValueProviderHTTPexists(mqttsettingsarray[1].as<String>(),mqttsettingsarray[2].as<int>(),exists_side,exists_primsec);
            if(!result) {
              m_valueProvider[side][0]=std::unique_ptr<ValueProviderHTTP>(new ValueProviderHTTP(mqttsettingsarray[1].as<String>(),
                                                                                                mqttsettingsarray[2].as<int>(),
                                                                                                jsonkeys,
                                                                                                mqttsettingsarray[4].as<double>(),
                                                                                                mqttsettingsarray[5].as<double>(),
                                                                                                std::bind(&DoubleBarController::displayValue, &this->barController, side, std::placeholders::_1),
                                                                                                primarysetting[3].as<int>()));
            } else {
              ValueProviderHTTP* valueprovider=(ValueProviderHTTP*)(m_valueProvider[exists_side][exists_primsec].get());
              valueprovider->addJSONkeys(jsonkeys,
                                         mqttsettingsarray[4].as<double>(),
                                         mqttsettingsarray[5].as<double>(),
                                         std::bind(&DoubleBarController::displayValue, &this->barController, side, std::placeholders::_1));
            }
          }
        } else if(primarysettingstring.equalsIgnoreCase("test")) {
          JsonArray& valuesarray=primarysetting[1].as<JsonArray>();
          m_valueProvider[side][0]=std::unique_ptr<ValueProviderTest>(new ValueProviderTest(valuesarray[0].as<double>(),
                                                                                            valuesarray[valuesarray.size()-1].as<double>(),
                                                                                            12*2,
                                                                                            std::bind(&DoubleBarController::displayValue, &this->barController, side, std::placeholders::_1),
                                                                                            primarysetting[3].as<int>()));
        }

        String secondarysettingstring=secondarysetting[0].as<String>();
        if(secondarysettingstring.equalsIgnoreCase("none")) {
          m_valueProvider[side][1]=nullptr;
          JsonArray& valuesarray=primarysetting[1].as<JsonArray>();
          JsonArray& colorsarray=primarysetting[2].as<JsonArray>();

          configureDisplay(side, valuesarray,colorsarray);
        } else {
          JsonArray& primaryvaluesarray=primarysetting[1].as<JsonArray>();
          JsonArray& secondaryvaluesarray=secondarysetting[1].as<JsonArray>();
          JsonArray& secondarycolorsarray=secondarysetting[2].as<JsonArray>();

          configureDisplay(side, primaryvaluesarray, secondaryvaluesarray, secondarycolorsarray);

          if(secondarysettingstring.equalsIgnoreCase("temperature") || secondarysettingstring.equalsIgnoreCase("humidity") || secondarysettingstring.equalsIgnoreCase("eCO2") || secondarysettingstring.equalsIgnoreCase("TVOC")) {
            ValueProviderSensor::SensorSource ssource;
            if(secondarysettingstring.equalsIgnoreCase("temperature")) {
              ssource=ValueProviderSensor::SensorSource::Temperature;
            } else if (secondarysettingstring.equalsIgnoreCase("humidity")) {
              ssource=ValueProviderSensor::SensorSource::Humidity;
            } else if (secondarysettingstring.equalsIgnoreCase("eCO2")) {
              ssource=ValueProviderSensor::SensorSource::eCO2;
            } else if (secondarysettingstring.equalsIgnoreCase("TVOC")) {
              ssource=ValueProviderSensor::SensorSource::TVOC;
            }

            m_valueProvider[side][1]=std::unique_ptr<ValueProviderSensor>(new ValueProviderSensor(sensors,
                                                                                                  ssource,
                                                                                                  std::bind(&DoubleBarController::displaySecondaryValue, &this->barController, side, std::placeholders::_1),
                                                                                                  secondarysetting[3].as<int>()));
          } else if(secondarysettingstring.equalsIgnoreCase("MQTT")) {
            if(secondarysetting[4].is<JsonArray>() && secondarysetting[4].as<JsonArray>()[0].as<String>().equalsIgnoreCase("MQTT")) {
              JsonArray& mqttsettingsarray=secondarysetting[4].as<JsonArray>();

              m_valueProvider[side][1]=std::unique_ptr<ValueProviderMQTT>(new ValueProviderMQTT(controller,
                                                                                                mqttsettingsarray[1].as<String>(),
                                                                                                mqttsettingsarray[2].as<double>(),
                                                                                                mqttsettingsarray[3].as<double>(),
                                                                                                std::bind(&DoubleBarController::displaySecondaryValue, &this->barController, side, std::placeholders::_1)));
            }
          } else if(secondarysettingstring.equalsIgnoreCase("HTTP")) {
            if(secondarysetting[4].is<JsonArray>() && secondarysetting[4].as<JsonArray>()[0].as<String>().equalsIgnoreCase("HTTP")) {
              JsonArray& mqttsettingsarray=secondarysetting[4].as<JsonArray>();

              std::vector<String> jsonkeys;
              if(mqttsettingsarray[3].is<JsonArray>()) {
                JsonArray& keyarray=mqttsettingsarray[3].as<JsonArray>();
                for(unsigned int i=0; i<keyarray.size(); i++) {
                  jsonkeys.push_back(keyarray[i].as<String>());
                }
              } else if(mqttsettingsarray[3].is<char*>()) {
                jsonkeys.push_back(mqttsettingsarray[3].as<String>());
              }
              int exists_side=0; int exists_primsec=0;
              bool result=ValueProviderHTTPexists(mqttsettingsarray[1].as<String>(),mqttsettingsarray[2].as<int>(),exists_side,exists_primsec);
              if(!result) {
                m_valueProvider[side][1]=std::unique_ptr<ValueProviderHTTP>(new ValueProviderHTTP(mqttsettingsarray[1].as<String>(),
                                                                                                  mqttsettingsarray[2].as<int>(),
                                                                                                  jsonkeys,
                                                                                                  mqttsettingsarray[4].as<double>(),
                                                                                                  mqttsettingsarray[5].as<double>(),
                                                                                                  std::bind(&DoubleBarController::displaySecondaryValue, &this->barController, side, std::placeholders::_1),
                                                                                                  secondarysetting[3].as<int>()));
              } else {
                ValueProviderHTTP* valueprovider=(ValueProviderHTTP*)(m_valueProvider[exists_side][exists_primsec].get());
                valueprovider->addJSONkeys(jsonkeys,
                                           mqttsettingsarray[4].as<double>(),
                                           mqttsettingsarray[5].as<double>(),
                                           std::bind(&DoubleBarController::displaySecondaryValue, &this->barController, side, std::placeholders::_1));
              }
            }
          } else if(secondarysettingstring.equalsIgnoreCase("test")) {
            JsonArray& valuesarray=secondarysetting[1].as<JsonArray>();
            m_valueProvider[side][1]=std::unique_ptr<ValueProviderTest>(new ValueProviderTest(valuesarray[0].as<double>(),
                                                                                              valuesarray[valuesarray.size()-1].as<double>(),
                                                                                              50,
                                                                                              std::bind(&DoubleBarController::displaySecondaryValue, &this->barController, side, std::placeholders::_1),
                                                                                              secondarysetting[3].as<int>()));
          }
        }
      }

      settingsFile.close();
    }
  }

  for(int i=0; i<2; i++) {
    for(int side=0; side<2; side++) {
      DoubleBarController::BarSide sideEn=(side==0 ? DoubleBarController::BarSide::Left : DoubleBarController::BarSide::Right);
      if(m_valueProvider[side][i]!=nullptr && m_valueProvider[side][i]->getType()==ValueProviderType::Test) {
        myLogger.addToLog(F("ValueProvider test:"));
        ValueProviderTest* provider=(ValueProviderTest*)(m_valueProvider[side][i].get());

        for(double value=provider->getLowerBound(); value<=provider->getUpperBound(); value+=provider->getDelta()) {
          if(i==0) {
            myLogger.addToLog("value: "+String(value)+" property: "+String(barController.getPrimaryProperty(sideEn,value)));
          } else {
            myLogger.addToLog("value: "+String(value)+" property: "+barController.getSecondaryProperty(sideEn,value).toString());
          }
        }
      }
    }
  }

  return true;
}

bool ThermoLight::ValueProviderHTTPexists(String url, int port, int &ret_side, int &ret_primsec)
{
  for(int side=0; side<2; side++) {
    for(int primsec=0; primsec<2; primsec++) {
      if(m_valueProvider[side][primsec]!=nullptr && m_valueProvider[side][primsec]->getType()==ValueProviderType::HTTP) {
        ValueProviderHTTP* valueprovider=(ValueProviderHTTP*)(m_valueProvider[side][primsec].get());
        if(valueprovider->getURL().equals(url) && valueprovider->getPort()==port) {
          ret_side=side;
          ret_primsec=primsec;
          return true;
        }
      }
    }
  }
  return false;
}

void ThermoLight::handleGetConfig(AsyncWebServerRequest *request, const char* jsonfilename, String subsetting)
{
  myLogger.addToLog(F("Getting config..."));

  if(subsetting.equals("")) {
    if(SPIFFS.exists(jsonfilename)) {
      request->send(SPIFFS, jsonfilename,"application/json");
    } else {
      request->send(200,"application/json", "{}");
    }

    return;
  } else {
    File settingsFile=SPIFFS.open(jsonfilename,"r");

    DynamicJsonBuffer jsonBufferSettings;
    JsonObject& rootSettings = jsonBufferSettings.parseObject(settingsFile);

    if(rootSettings.containsKey(subsetting)) {
      AsyncResponseStream *response = request->beginResponseStream("application/json");
      rootSettings[subsetting].printTo(*response);
      request->send(response);

      return;
    }
  }

  request->send(404);
  return;
}

void ThermoLight::handleSetConfigGeneric(JsonObject &settingstoedit, JsonArray &newsettings, bool isPrimary)
{
  settingstoedit.remove(isPrimary ? "primary" : "secondary");
  JsonArray& arrayToEdit=settingstoedit.createNestedArray(isPrimary ? "primary" : "secondary");
  if(arrayToEdit==JsonArray::invalid()) {
    myLogger.addToLog(F("Couldn't allocate new JSON array."));
  }

  for(unsigned int i=0; i<newsettings.size(); i++) {
    arrayToEdit.add(newsettings[i]);
  }
}

void ThermoLight::handleSetConfig(AsyncWebServerRequest *request, bool isLeft, bool isPrimary)
{
  if(request->method()!=HTTP_PUT || !request->hasHeader("Content-Type") || request->header("Content-Type").indexOf("application/json")==-1) {
    request->send(400);
    return;
  }

  DynamicJsonBuffer jsonBuffer;
  JsonArray& root = jsonBuffer.parseArray((char*)request->_tempObject);

  if(root==JsonArray::invalid()) {
    // error parsing config file
    request->send(400);
    return;
  } else {
    File sourcesFile=SPIFFS.open(m_settingsfilename[SettingsType::Sources],"r");

    DynamicJsonBuffer jsonBufferSources;
    JsonArray& sources = jsonBufferSources.parseArray(sourcesFile);

    if(sources==JsonArray::invalid()) {
      // error parsing config file
      myLogger.addToLog(F("Can't read sources file."));
      request->send(500);
      sourcesFile.close();

      return;
    }

    if(!validateSettings(root, sources)) {
      request->send(422);
      sourcesFile.close();

      return;
    }

    sourcesFile.close();
  }

  File settingsFile;

  if(isLeft) {
    settingsFile=SPIFFS.open(m_settingsfilename[SettingsType::Left],"r");
  } else {
    settingsFile=SPIFFS.open(m_settingsfilename[SettingsType::Right],"r");
  }

  DynamicJsonBuffer jsonBufferSettings;
  JsonObject& rootSettings = jsonBufferSettings.parseObject(settingsFile);

  if(rootSettings==JsonObject::invalid()) {
    // error parsing config file
    myLogger.addToLog(F("Can't read config."));
    request->send(500);
    settingsFile.close();

    return;
  }

  handleSetConfigGeneric(rootSettings,root,isPrimary);

  File outputFile=SPIFFS.open("/temp.dat","w");
  rootSettings.prettyPrintTo(outputFile);

  outputFile.close();
  settingsFile.close();

  SPIFFS.remove(m_settingsfilename[isLeft ? SettingsType::Left : SettingsType::Right]);
  SPIFFS.rename("/temp.dat",m_settingsfilename[isLeft ? SettingsType::Left : SettingsType::Right]);
  myLogger.addToLog(F("Settings saved."));

  request->send(200);

  m_reloadsettings=true;

  return;
}

bool ThermoLight::validateBrightness(JsonArray &newsettings, JsonArray &brightnesssources)
{
  if(newsettings[0].is<char*>() && brightnesssources!=JsonArray::invalid()) {
    bool found=false;
    for(unsigned int i=0; i<brightnesssources.size(); i++) {
      if(brightnesssources[i].is<char*>()) {
        if(brightnesssources[i].as<String>()==newsettings[0].as<String>()) {
          found=true;
          break;
        }
      }
    }
    if(!found) return false;
  }

  String sourcetype=newsettings[0].as<String>();

  if(sourcetype.equalsIgnoreCase("fixed")) {
    if(newsettings.size()!=2) {
      return false;
    }

    if(!newsettings[1].is<double>()) {
      return false;
    }
  }

  if(sourcetype.equalsIgnoreCase("MQTT")) {
    if(newsettings.size()!=7) {
      return false;
    }

    if(!newsettings[1].is<char*>() || !newsettings[2].is<double>() || !newsettings[3].is<double>() || !newsettings[4].is<double>() || !newsettings[5].is<double>() || !newsettings[6].is<bool>()) {
      return false;
    }
  }

  return true;
}

bool ThermoLight::validateSettings(JsonArray &newsettings, JsonArray & sources)
{
  if(newsettings.size()!=4 && newsettings.size()!=5) {
    return false;
  }

  if(newsettings[0].is<char*>() && sources!=JsonArray::invalid()) {
    bool found=false;
    for(unsigned int i=0; i<sources.size(); i++) {
      if(sources[i].is<char*>()) {
        if(sources[i].as<String>()==newsettings[0].as<String>()) {
          found=true;
          break;
        }
      }
    }
    if(!found) return false;
  }

  if(!newsettings[1].is<JsonArray>() || !newsettings[2].is<JsonArray>() || !newsettings[3].is<double>()) {
    return false;
  }
  JsonArray& valuearray=newsettings[1].as<JsonArray>();
  JsonArray& colorarray=newsettings[2].as<JsonArray>();

  if(valuearray.size()!=colorarray.size()) {
    return false;
  }

  for(unsigned int i=0; i<valuearray.size(); i++) {
    if(!valuearray[i].is<double>() || !colorarray[i].is<JsonArray>()) {
      return false;
    }

    JsonArray& subcolorarray=colorarray[i].as<JsonArray>();
    if((!subcolorarray.size()==3) ||
       (!subcolorarray[0].is<unsigned int>() || !subcolorarray[1].is<unsigned int>() || !subcolorarray[2].is<unsigned int>())) {
         return false;
    }
  }

  if(newsettings.size()==5) {
    if(!newsettings[4].is<JsonArray>()) {
      return false;
    }

    JsonArray& newsettingssub=newsettings[4].as<JsonArray>();
    if(!newsettingssub[0].is<char*>()) {
      return false;
    }

    String subtype=newsettingssub[0].as<String>();
    if(subtype.equalsIgnoreCase("HTTP")) {
      if(!newsettingssub[1].is<char*>() || !newsettingssub[2].is<unsigned int>() || !newsettingssub[3].is<JsonArray>() || !newsettingssub[4].is<double>() || !newsettingssub[5].is<double>()) {
        return false;
      }
      for(unsigned int i=0; i<newsettingssub[3].as<JsonArray>().size(); i++) {
        if(!newsettingssub[3].as<JsonArray>()[i].is<char*>()) {
          return false;
        }
      }
    } else if(subtype.equalsIgnoreCase("MQTT")) {
      if(!newsettingssub[1].is<char*>() || !newsettingssub[2].is<double>() || !newsettingssub[3].is<double>()) {
        return false;
      }
    } else {
      return false;
    }
  }

  return true;
}

void ThermoLight::handleSetBrightness(AsyncWebServerRequest *request)
{
  if(request->method()!=HTTP_PUT || !request->hasHeader("Content-Type") || request->header("Content-Type").indexOf("application/json")==-1) {
    request->send(400);
    return;
  }

  DynamicJsonBuffer jsonBuffer;
  JsonArray& root = jsonBuffer.parseArray((char*)request->_tempObject);

  if(root==JsonArray::invalid()) {
    // error parsing request body
    request->send(400);
    return;
  } else {
    File sourcesFile=SPIFFS.open(m_settingsfilename[SettingsType::SourcesBrightness],"r");

    DynamicJsonBuffer jsonBufferSources;
    JsonArray& sources = jsonBufferSources.parseArray(sourcesFile);

    if(sources==JsonArray::invalid()) {
      // error parsing config file
      myLogger.addToLog(F("Can't read sources file."));
      request->send(500);
      sourcesFile.close();

      return;
    }

    if(!validateBrightness(root, sources)) {
      request->send(422);
      sourcesFile.close();

      return;
    }

    sourcesFile.close();
  }

  File outputFile=SPIFFS.open("/temp.dat","w");
  root.prettyPrintTo(outputFile);
  outputFile.close();

  SPIFFS.remove(m_settingsfilename[SettingsType::Brightness]);
  SPIFFS.rename("/temp.dat",m_settingsfilename[SettingsType::Brightness]);
  myLogger.addToLog(F("Settings saved."));

  request->send(200);

  m_reloadsettings=true;

  return;
}

void ThermoLight::handleGetColorScale(AsyncWebServerRequest * request)
{
  if(request->method()!=HTTP_GET || !request->hasArg("color1") || !request->hasArg("color2")) {
      myLogger.addToLog(F("Wrong HTTP method or server has no arg 'color1' or 'color2''."));
      request->send(400);
      return;
  }

  if(request->arg("color1").length()!=6 || request->arg("color2").length()!=6) {
    request->send(422);
    return;
  }

  int upperlimit=12;
  if(request->hasArg("num")) {
    upperlimit=std::strtoul(request->arg("num").c_str(),nullptr,10);
  }

  String scolor1=String(request->arg("color1"));
  String scolor2=String(request->arg("color2"));

  ColorHelper::RGB color1={(uint8_t) std::strtoul(scolor1.substring(0, 2).c_str(),nullptr,16), (uint8_t) std::strtoul(scolor1.substring(2, 4).c_str(),nullptr,16), (uint8_t) std::strtoul(scolor1.substring(4, 6).c_str(),nullptr,16)};
  ColorHelper::RGB color2={(uint8_t) std::strtoul(scolor2.substring(0, 2).c_str(),nullptr,16), (uint8_t) std::strtoul(scolor2.substring(2, 4).c_str(),nullptr,16), (uint8_t) std::strtoul(scolor2.substring(4, 6).c_str(),nullptr,16)};

  request->send(200, "application/json", getColorString(color1,color2, upperlimit));
}

String ThermoLight::getColorString(ColorHelper::RGB color1, ColorHelper::RGB color2, int upperlimit)
{
  ColorHelper::Scale<ColorHelper::LCH> scale(0,ColorHelper::RGBtoLCH(color1),upperlimit-1,ColorHelper::RGBtoLCH(color2));

  String retval="[\"";
  for(int i=0; i<upperlimit; i++) {
    retval+=ColorHelper::LCHtoRGB(scale.getProperty(i)).toHexString();
    if(i<upperlimit-1) {
      retval+="\",\"";
    }
  }
  retval+="\"]";

  return retval;
}

void ThermoLight::handleGetSensorValues(AsyncWebServerRequest * request)
{
  String retval="{";
  retval+=String("\"temperature\":")+String(sensors.getTemperature());
  retval+=String(",\"humidity\":")+String(sensors.getHumidity());
  retval+=String(",\"tvoc\":")+String(sensors.getTVOC());
  retval+=String(",\"eco2\":")+String(sensors.geteCO2());
  retval+=String("}");

  request->send(200, "application/json", retval);
}

void ThermoLight::sendNewSensorValue(String mqtttopic, String sensoreventstring, double value)
{
  if(mqtttopic!="") {
    controller.publishMQTT(mqtttopic, String(value));
  }

  if(sensoreventstring!="") {
    controller.sendEvent("SensorValue", "{\""+sensoreventstring+"\":"+String(value)+"}");
  }
}

void ThermoLight::handleGetConfigAccumulated(AsyncWebServerRequest * request)
{
  if(request->_tempObject==nullptr) {
    std::vector<String> includepaths;
    includepaths.push_back("/config/");
    std::vector<String> filesblacklist;
    filesblacklist.push_back("/config/webserver.json");
    request->_tempObject=new FilesToJSON(includepaths,filesblacklist,false,true);
  }

  request->onDisconnect([request]() {
    delete ((FilesToJSON*)request->_tempObject);
    request->_tempObject=nullptr;
  });

  AsyncWebServerResponse *response=request->beginChunkedResponse("application/json", [request](uint8_t *buffer, size_t maxlen, size_t index) -> size_t {
    size_t bytesWritten=((FilesToJSON*)request->_tempObject)->continueFiles(buffer, maxlen);
    myLogger.addToLog(F("ChunkedResponse: "),String(bytesWritten));

    return bytesWritten;
  });

  request->send(response);

  return;
}

void ThermoLight::handleSetConfigAccumulated(AsyncWebServerRequest * request)
{
  if(request->method()!=HTTP_PUT || !request->hasHeader("Content-Type") || request->header("Content-Type").indexOf("application/json")==-1) {
    request->send(400);
    return;
  }

  // object seems to be valid, save new settings to temporary file
  File tempFile=SPIFFS.open("/newset.dat","w");
  tempFile.print((char*)request->_tempObject);
  //root.printTo(tempFile);
  tempFile.close();
  m_writenewsettings=true;

  request->send(200);

  return;
}

void ThermoLight::mqttCallback(String, std::unique_ptr<char []>)
{
}

void ThermoLight::setBrightness(double value, double lowerbound, double upperbound, bool zeroiflower)
{
  barController.setBrightness(value>upperbound ? upperbound : (value<lowerbound ? (zeroiflower ? 0 : lowerbound) : value));
}

void ThermoLight::writeNewSettings()
{
  File newsettingsFile=SPIFFS.open("/newset.dat","r");
  std::array<String,3> keys={"\"brightness\":","\"left\":","\"right\":"};
  std::array<SettingsType,3> keytypes={SettingsType::Brightness, SettingsType::Left, SettingsType::Right};

  for(uint8_t i=0; i<keys.size(); i++) {
    newsettingsFile.seek(0,SeekSet);
    if(!newsettingsFile.find(keys[i].c_str())) continue;

    myLogger.addToLog(F("Found key "),keys[i]);
    myLogger.addToLog(F("Heap"),String(ESP.getFreeHeap()));
    DynamicJsonBuffer jsonBufferNewSettings;
    myLogger.addToLog(F("Heap"),String(ESP.getFreeHeap()));
    if(i==0) {
      JsonArray& root = jsonBufferNewSettings.parseArray(newsettingsFile);

      if(root==JsonArray::invalid()) {
        myLogger.addToLog(F("Failed to parse"));
        continue;
      } else {
        File sourcesFile=SPIFFS.open(m_settingsfilename[SettingsType::SourcesBrightness],"r");

        DynamicJsonBuffer jsonBufferSources;
        JsonArray& sources = jsonBufferSources.parseArray(sourcesFile);

        if(sources==JsonArray::invalid()) {
          // error parsing config file
          myLogger.addToLog(F("Can't read sources file."));
          sourcesFile.close();

          continue;
        }

        if(!validateBrightness(root, sources)) {
          myLogger.addToLog(F("Can't validate new brightness settings."));
          sourcesFile.close();

          continue;
        }

        sourcesFile.close();
      }

      File outputFile=SPIFFS.open(m_settingsfilename[keytypes[i]],"w");
      root.prettyPrintTo(outputFile);
      outputFile.close();

      myLogger.addToLog(F("Data written"));
    } else {
      JsonObject& root = jsonBufferNewSettings.parseObject(newsettingsFile);

      DynamicJsonBuffer jsonBufferSettings;
      File settingsFile=SPIFFS.open(m_settingsfilename[keytypes[i]],"r");
      JsonObject& rootSettings = jsonBufferSettings.parseObject(settingsFile);

      if(root==JsonObject::invalid() || rootSettings==JsonObject::invalid()) {
        myLogger.addToLog(F("Failed to parse."));
        settingsFile.close();

        continue;
      }

      bool modified=false;
      if(root.containsKey("primary") && root["primary"].is<JsonArray>()) {
        JsonArray& sourceArray=root["primary"].as<JsonArray>();
        if(validateSettings(sourceArray, JsonArray::invalid())) {
          rootSettings.remove("primary");
          JsonArray& arrayToEdit=rootSettings.createNestedArray("primary");
          for(unsigned int i=0; i<sourceArray.size(); i++) {
            arrayToEdit.add(sourceArray[i]);
          }
          myLogger.addToLog(F("Primary written"));
          modified=true;
        }
      }
      if(root.containsKey("secondary") && root["secondary"].is<JsonArray>()) {
        JsonArray& sourceArray=root["secondary"].as<JsonArray>();
        if(validateSettings(sourceArray, JsonArray::invalid())) {
          rootSettings.remove("secondary");
          JsonArray& arrayToEdit=rootSettings.createNestedArray("secondary");
          for(unsigned int i=0; i<sourceArray.size(); i++) {
            arrayToEdit.add(sourceArray[i]);
          }
          myLogger.addToLog(F("Secondary written"));
          modified=true;
        }
      }

      settingsFile.close();

      if(modified) {
        File outputFile=SPIFFS.open(m_settingsfilename[keytypes[i]],"w");
        rootSettings.prettyPrintTo(outputFile);
        outputFile.close();
      }
    }
    myLogger.addToLog(F("Heap"),String(ESP.getFreeHeap()));
  }

  newsettingsFile.close();

  SPIFFS.remove("/newset.dat");
  myLogger.addToLog(F("Settings saved."));

  m_reloadsettings=true;
}
