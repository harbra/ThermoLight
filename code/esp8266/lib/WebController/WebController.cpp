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

#include "WebController.h"

WebController::WebController(String AppName) : pathprefix("/html"), server(80), events("/events"), previousMillisConnectionCheck(0), timeoutConnectionCheck(5000), resetWifi(false), reboot(false), restoreSettingsFile(""), appname(AppName), mqtt_enabled(false), mqtt_connect(false), mqtt_lastattempt(0), mqtt_wificlient(nullptr), mqtt_pubsubclient(nullptr), ota_server(nullptr) //, mqtt_cert(nullptr)
{
}

void WebController::setup()
{
        myLogger.addToLog(F("Starting..."));
        SPIFFS.begin();
        SPIFFS.remove("/temp/temp");

        settingsfilename="/config/webserver.json";
        loadSettings(settingsfilename);
        resetWifiConfig();

        resetSettingsIncludePaths();
        resetSettingsBlacklist();

        setupServer();

        WiFi.scanNetworks(true);
}

void WebController::loop()
{
        dnsserver.processNextRequest();
        if(mqtt_pubsubclient!=nullptr) {
          mqtt_pubsubclient->loop();
        }

        if(ota_server!=nullptr)
        {
          ota_server->handle();
        }

        if(mqtt_enabled && WiFi.status()==WL_CONNECTED)
        {
          if((millis()-mqtt_lastattempt>5000) &&
             ((mqtt_pubsubclient==nullptr) ||
              (mqtt_pubsubclient!=nullptr && !mqtt_pubsubclient->connected()))) {
            mqtt_lastattempt=millis();
            reconnectMQTT();
          }
        }

        if(resetWifi)
        {
          resetWifiConfig();
          resetWifi=false;
        }

        if(restoreSettingsFile!="")
        {
          restoreSettings();
        }

        if(reboot)
        {
          reboot=false;
          delay(2500);
          ESP.restart();
        }

        if(millis()-previousMillisConnectionCheck>=timeoutConnectionCheck)
        {
          previousMillisConnectionCheck=millis();
          sendEvent("ConnectionCheck",getConnectionStatus());
        }
}

String WebController::getConnectionStatus()
{
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  root["isConnected"]=WiFi.isConnected();
  root["FreeHeap"]=ESP.getFreeHeap();
  //root["HeapFragmentation"]=ESP.getHeapFragmentation();
  if(WiFi.isConnected())
  {
      root["RSSI"]=WiFi.RSSI();
      root["SSID"]=WiFi.SSID();
      if(WiFi.psk().length()>0)
      {
        root["encrypted"]=true;
      } else
      {
        root["encrypted"]=false;
      }
  }

  String output;
  root.printTo(output);

  return output;
}

void WebController::sendEvent(const String& eventname, const String& eventcontent, bool muteserial)
{
  if(!muteserial) myLogger.addToLog(F("SendEvent: "),eventcontent);
  events.send(eventcontent.c_str(),eventname.c_str());
}

void WebController::setupServer()
{
      // set up html server
        myLogger.addToLog(F("Setting up server..."));
        server.on("/api/test",HTTP_POST,std::bind(&WebController::handleTest, this, std::placeholders::_1),std::bind(&WebController::handleFileUploadDummy, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6),std::bind(&WebController::handleRequestBody, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
        server.on("/api/settings",HTTP_GET,std::bind(&WebController::handleGetSettings, this, std::placeholders::_1));
        server.on("/api/settings",HTTP_POST,std::bind(&WebController::handleSetSettings, this, std::placeholders::_1),std::bind(&WebController::handleFileUploadDummy, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6),std::bind(&WebController::handleRequestBody, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
        server.on("/api/scanwifi",HTTP_GET,std::bind(&WebController::handleScanWifi, this, std::placeholders::_1));
        server.on("/api/getinfo",HTTP_GET,std::bind(&WebController::handleGetInfo, this, std::placeholders::_1));
        server.on("/api/getwifistatus",HTTP_GET,std::bind(&WebController::handleGetWifiStatus, this, std::placeholders::_1));
        server.on("/api/reboot",HTTP_GET,std::bind(&WebController::handleReboot, this, std::placeholders::_1));
        server.on("/api/downloadsettings",HTTP_GET,std::bind(&WebController::handleDownloadSettings, this, std::placeholders::_1));
        server.on("/api/restoresettings",HTTP_POST,std::bind(&WebController::handleRestoreSettings, this, std::placeholders::_1),std::bind(&WebController::handleFileUpload, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6),std::bind(&WebController::handleRequestBody, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
        server.on("/api/ota/enable",HTTP_GET,std::bind(&WebController::handleEnableOTA, this, std::placeholders::_1));
        server.on("/api/ota/disable",HTTP_GET,std::bind(&WebController::handleDisableOTA, this, std::placeholders::_1));
        server.on("/api/ota",HTTP_GET,std::bind(&WebController::handleOTAStatus, this, std::placeholders::_1));
        server.on("/api/serial/enable", HTTP_GET, std::bind(&WebController::handleEnableSerial, this, std::placeholders::_1));
        server.on("/api/serial/disable", HTTP_GET, std::bind(&WebController::handleDisableSerial, this, std::placeholders::_1));
        server.on("/api/serial", HTTP_GET, std::bind(&WebController::handleGetSerialStatus, this, std::placeholders::_1));

        // server content from SPIFFS in case no route already caught this URI
        server.onNotFound([this](AsyncWebServerRequest *request){
                                  bool handled=false;
                                  for(WebHandlerContainer myWebHandler : additionalWebHandlers) {
                                    if(request->url().startsWith(myWebHandler.myUri) && request->method()==myWebHandler.myMethod) {
                                      handled=true;
                                      myWebHandler.myOnRequest(request,request->url().substring(myWebHandler.myUri.length()));
                                    }
                                  }
                                  if(!handled) {
                                    if(request->url().startsWith("/api")) {
                                      // special processing for api requests
                                      request->send(404);
                                    } else if(!handleFileRead(request, request->url()))
                                    {
                                        handleFileRead(request, "/");  // in case no file with this URI is found, return the standard index.html file. Needed for vue.js in history mode.
                                    }
                                  }
                          });

        server.onRequestBody(std::bind(&WebController::handleRequestBody, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));

        // set up server-sent events server
        events.onConnect([](AsyncEventSourceClient *client){
          if(client->lastId()){
            // client reconnected
          }
          //client->send("Hello!",NULL,millis(),1000);
        });

        server.addHandler(&events);

        server.begin();
}

void WebController::resetWifiConfig()
{
  if(!connectWifi()) {
    myLogger.addToLog(F("Couldn't connect to an existing AP. Setting one up."));
      createAP();
  }

  // configure mdns
  if(!MDNS.begin(ap_ssid)) {
    myLogger.addToLog(F("Couldn't activate MDNS responder."));
  } else {
    myLogger.addToLog(F("MDNS responder activated at: "),String(ap_ssid));
  }

  // configure ntp
  if(ntp_enabled) {
    myLogger.addToLog(F("enabling NTP"));
    configTime(0,0,ntp_server1,ntp_server2,ntp_server3);
  } else {
    myLogger.addToLog(F("disabling NTP"));
    configTime(0,0,"","","");
  }
}

bool WebController::connectWifi()
{
        dnsserver.stop();

        WiFi.mode(WIFI_STA);

        if(!sta_dhcp) {
          IPAddress IP_sta_ip; IPAddress IP_sta_gw; IPAddress IP_sta_sn;

          if(IP_sta_ip.fromString(sta_ip) &&
             IP_sta_gw.fromString(sta_gw) &&
             IP_sta_sn.fromString(sta_sn)) {
               WiFi.config(IP_sta_ip,IP_sta_gw,IP_sta_sn);
             } else {
               myLogger.addToLog(F("Invalid static IP configuration. Using DHCP."));
             }
        }

        if(WiFi.status()==WL_CONNECTED) {
          myLogger.addToLog(F("Connected."));
          return true;
        }

        if(sta_ssid==NULL || strlen(sta_ssid)==0)
        {
          myLogger.addToLog(F("No SSID given for AP. Cancelling."));
          return false;
        }

        if(strlen(sta_pwd)>=8 && strlen(sta_pwd)<=63) {
          myLogger.addToLog(F("Connecting Wifi. SSID: "), sta_ssid);
          myLogger.addToLog(F("PW: "), sta_pwd);
            WiFi.begin(sta_ssid, sta_pwd);
        } else {
          myLogger.addToLog(F("Connecting Wifi without password. SSID: "), sta_ssid);
          WiFi.begin(sta_ssid);
        }

        if(WiFi.waitForConnectResult() == WL_CONNECTED) {
          myLogger.addToLog(F("Connected."));
          return true;
        } else {
          myLogger.addToLog(F("Failed to connect."));
          return false;
        }
}

void WebController::createAP()
{
        myLogger.addToLog(F("Setting up access point..."));
        WiFi.mode(WIFI_AP_STA);
        //WiFi.softAPConfig(IPAddress(192,168,1,1),IPAddress(192,168,1,1),IPAddress(255,255,255,0));
        WiFi.softAP(ap_ssid,ap_pwd);

        dnsserver.setErrorReplyCode(DNSReplyCode::NoError);
        dnsserver.start(53,"*",IPAddress(192,168,4,1));
}

String WebController::getContentType(String filename)
{
        if(filename.endsWith(".htm")) return "text/html";
        else if(filename.endsWith(".html")) return "text/html";
        else if(filename.endsWith(".css")) return "text/css";
        else if(filename.endsWith(".js")) return "application/javascript";
        else if(filename.endsWith(".png")) return "image/png";
        else if(filename.endsWith(".gif")) return "image/gif";
        else if(filename.endsWith(".jpg")) return "image/jpeg";
        else if(filename.endsWith(".ico")) return "image/x-icon";
        else if(filename.endsWith(".xml")) return "text/xml";
        else if(filename.endsWith(".pdf")) return "application/x-pdf";
        else if(filename.endsWith(".zip")) return "application/x-zip";
        else if(filename.endsWith(".gz")) return "application/x-gzip";
        return "text/plain";
}

void WebController::handleReboot(AsyncWebServerRequest * request)
{
  reboot=true;
}

void WebController::handleGetWifiStatus(AsyncWebServerRequest * request)
{
  request->send(200,"application/json",getConnectionStatus());
}

void WebController::handleRequestBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total)
{
  if(request->_tempObject==nullptr) {
    request->_tempObject=malloc(total+1);
    ((uint8_t*)request->_tempObject)[total]='\0';
  }

  for(size_t i=0; i<len; i++){
    ((uint8_t*)request->_tempObject)[index+i]=data[i];
  }
}

void WebController::handleFileUploadDummy(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
}

void WebController::handleFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  File myFile=SPIFFS.open("/temp/"+filename,"a");

  myLogger.addToLog(F("Writing to file: "),myFile.name());

  myLogger.addToLog(F("seek: "),String(std::min(myFile.size(),index)));
  myFile.seek(std::min(myFile.size(),index));
  myLogger.addToLog(F("position: "),String(myFile.position()));
  myLogger.addToLog(F("index: "),String(index));

  while(myFile.position()<index) {
    myFile.write(0);
  }

  myLogger.addToLog(F("length: "),String(len));
  myLogger.addToLog(F("written: "),String(myFile.write(data,len)));

  myFile.close();
}

void WebController::handleDownloadSettings(AsyncWebServerRequest * request)
{
  if(request->_tempObject==nullptr) {
    request->_tempObject=new FilesToJSON(settings_includepaths,settings_filesblacklist);
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
}

void WebController::handleRestoreSettings(AsyncWebServerRequest * request)
{
  // check if a FILE was posted
  int params = request->params();
  for(int i=0;i<params;i++){
    AsyncWebParameter* p = request->getParam(i);
    if(p->isFile() && p->value()!=""){
      // TODO: Check if filenames are longer than 31 characters and do something if yes (not only here)
      myLogger.addToLog(F("Filename for restore: "),p->value());
      restoreSettingsFile="/temp/"+p->value();
      request->send(200);
    }
  }

  request->send(400);
}

void WebController::handleGetInfo(AsyncWebServerRequest *request)
{
  AsyncJsonResponse * response = new AsyncJsonResponse(false);
  //DynamicJsonBuffer jsonOutput;
  JsonObject& output=response->getRoot();

  output["chipid"]=ESP.getChipId();
  if(WiFi.isConnected()) {
    output["ip"]=WiFi.localIP().toString();
  } else {
    output["ip"]=WiFi.softAPIP().toString();
  }

  if(request->hasArg("comprehensive")) {
    if(WiFi.isConnected()) {
      output["isconnected"]=true;
      output["bssid"]=WiFi.BSSID();
      output["channel"]=WiFi.channel();
      output["gatewayip"]=WiFi.gatewayIP().toString();
      output["hostname"]=WiFi.hostname();
      output["macaddress"]=WiFi.macAddress();
      output["ssid"]=WiFi.SSID();
      output["subnetmask"]=WiFi.subnetMask().toString();
      if(WiFi.psk().length()>0)
      {
        output["encrypted"]=true;
      } else
      {
        output["encrypted"]=false;
      }
    } else {
      output["isconnected"]=false;
      output["softapip"]=WiFi.softAPIP().toString();
      output["softapmac"]=WiFi.softAPmacAddress();
    }
    output["cpufreqmhz"]=ESP.getCpuFreqMHz();
    output["flashchipid"]=ESP.getFlashChipId();
    output["flashchipsize"]=ESP.getFlashChipSize();
    output["flashchiprealsize"]=ESP.getFlashChipRealSize();
    output["flashchipspeed"]=ESP.getFlashChipSpeed();
    output["freeheap"]=ESP.getFreeHeap();
    //output["heapfragmentation"]=ESP.getHeapFragmentation();
    output["freesketchspace"]=ESP.getFreeSketchSpace();
    output["sketchsize"]=ESP.getSketchSize();
    output["millis"]=millis();
  }

  response->setLength();
  request->send(response);
}

void WebController::handleTest(AsyncWebServerRequest *request)
{
        myLogger.addToLog(F("url: "),String(request->url()));
        myLogger.addToLog(F("method: "), String(request->method()));
        myLogger.addToLog(F("headers: "), String(request->headers()));
        for (uint8_t i=0; i<request->headers(); i++) {
                myLogger.addToLog(request->headerName(i) + ": ",String(request->header(i)));
        }
        myLogger.addToLog(F("args: "), String(request->args()));
        for (uint8_t i=0; i<request->args(); i++) {
                myLogger.addToLog(request->argName(i) + ": ",String(request->arg(i)));
        }
        if(request->_tempObject!=nullptr)
        {
          myLogger.addToLog(F("body: "),String((char*)request->_tempObject));

          // memory is freed automatically in the request's destructor
          //free(request->_tempObject);
          //request->_tempObject=nullptr;
        }
        //List all parameters
        int params = request->params();
        for(int i=0;i<params;i++){
          AsyncWebParameter* p = request->getParam(i);
          if(p->isFile()){ //p->isPost() is also true
            Serial.printf("FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
          } else if(p->isPost()){
            Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
          } else {
            Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
          }
        }
        request->send(204);
}

bool WebController::handleScanWifi(AsyncWebServerRequest *request)
{
        AsyncJsonResponse * response = new AsyncJsonResponse(true);
        JsonArray& output=response->getRoot();

        int n = WiFi.scanComplete();
        if(n == -2){
          WiFi.scanNetworks(true);
        } else if(n){
          myLogger.addToLog(F("Found networks: "),String(n));
                for (int i = 0; i < n; ++i)
                {
                    JsonObject& output_network=output.createNestedObject();
                    output_network["ssid"]=WiFi.SSID(i);
                    output_network["rssi"]=WiFi.RSSI(i);
                    output_network["enc"]=WiFi.encryptionType(i);
                    output_network["channel"]=WiFi.channel(i);
                }
                WiFi.scanDelete();
                if(WiFi.scanComplete()==-2){
                  WiFi.scanNetworks(true);
                }

        }

        response->setLength();
        request->send(response);

        return true;
}

bool WebController::handleSetSettings(AsyncWebServerRequest *request)
{
        if(request->method()!=HTTP_POST || !request->hasHeader("Content-Type") || request->header("Content-Type").indexOf("application/json")==-1) {
                request->send(400);
                return false;
        }

        DynamicJsonBuffer jsonBuffer;
        JsonObject& root = jsonBuffer.parseObject((char*)request->_tempObject);

        if(root==JsonObject::invalid()) {
                // error parsing config file
                request->send(400);
                return false;
        }

        File settingsFile=SPIFFS.open(settingsfilename,"r");

        DynamicJsonBuffer jsonBufferSettings;
        JsonObject& rootSettings = jsonBufferSettings.parseObject(settingsFile);

        if(rootSettings==JsonObject::invalid()) {
                // error parsing config file
                myLogger.addToLog(F("Can't read config."));
                request->send(500);
                settingsFile.close();

                return false;
        }
        JsonObject& settings=rootSettings["settings"];

        for(auto kt : root)
        {
                if(settings.containsKey(kt.key)) {
                        JsonObject& subsettings=settings[kt.key];

                        for(auto it : kt.value.as<JsonObject>()) {
                                if(subsettings.containsKey(it.key)) {
                                        myLogger.addToLog(F("Found: "),it.key);
                                        myLogger.addToLog(F("Writing: "),it.value.as<String>());
                                        JsonArray& subsettingsvalue=subsettings[it.key];
                                        subsettingsvalue[0]=it.value;
                                } else {
                                        myLogger.addToLog(F("Couldn't find subsetting: "), it.key);
                                }
                        }
                } else {
                        myLogger.addToLog(F("Couldn't find setting: "),kt.key);
                }
        }

        File outputFile=SPIFFS.open("/temp.dat","w");
        rootSettings.prettyPrintTo(outputFile);

        outputFile.close();
        settingsFile.close();

        SPIFFS.remove(settingsfilename);
        SPIFFS.rename("/temp.dat",settingsfilename);
        myLogger.addToLog(F("Settings saved."));

        request->send(200);

        loadSettings(settingsfilename);
        resetWifi=true;

        return true;
}

bool WebController::handleGetSettings(AsyncWebServerRequest *request)
{
        if(request->method()!=HTTP_GET || !request->hasArg("type")) {
                myLogger.addToLog(F("Wrong HTTP method or server has no arg 'type'."));
                request->send(400);
                return false;
        }

        File settingsFile=SPIFFS.open(settingsfilename,"r");

        DynamicJsonBuffer jsonBuffer;
        JsonObject& root = jsonBuffer.parseObject(settingsFile);

        if(root==JsonObject::invalid()) {
                // error parsing config file
                myLogger.addToLog(F("Can't read config."));
                request->send(500);
                settingsFile.close();
                return false;
        }
        JsonObject& settings=root["settings"];

        if(!settings.containsKey(request->arg("type")) && request->arg("type")!="*") {
                myLogger.addToLog(F("Couldn't find key: "),request->arg("type"));
                request->send(500);
                settingsFile.close();
                return false;
        }

        AsyncJsonResponse * response = new AsyncJsonResponse();

        JsonObject& output=response->getRoot();
        for(auto kt : settings)
        {
                if(request->arg("type")=="*" || request->arg("type")==kt.key)
                {
                        JsonObject& suboutput=output.createNestedObject(kt.key);
                        JsonObject& currentSettings=kt.value.as<JsonObject>();

                        for(auto it : currentSettings)
                        {
                                JsonArray& settingValue=it.value.as<JsonArray>();
                                if(settingValue.size()==1 || settingValue[1]==true)
                                { // the array either only has one element, or the second element is true, which means the value should be displayed
                                        suboutput[it.key]=settingValue[0];
                                }
                        }
                }
        }

        response->setLength();
        request->send(response);
        settingsFile.close();
        return true;
}

bool WebController::handleFileRead(AsyncWebServerRequest *request, String path)
{
        myLogger.addToLog(F("Requesting: "),path);
        path=pathprefix+path;
        if(path.endsWith("/")) path += "index.html";
        String contentType = getContentType(path);
        String pathWithGz = path + ".gz";
        if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
                //if(SPIFFS.exists(pathWithGz))
                //        path += ".gz";

                myLogger.addToLog(F("serving: "), path);

                AsyncWebServerResponse *response = request->beginResponse(SPIFFS, path, contentType);

                //if(SPIFFS.exists(pathWithGz)) {
                //  response->addHeader("Content-Encoding", "x-gzip");
                //}

                request->send(response);

                return true;
        }
        myLogger.addToLog(F("404"));
        return false;
}

bool WebController::loadSettings(String filename)
{
        myLogger.addToLog(F("Loading settings..."));

        File settingsFile=SPIFFS.open(filename,"r");

        DynamicJsonBuffer jsonBuffer;
        JsonObject& root = jsonBuffer.parseObject(settingsFile);

        if(root==JsonObject::invalid()) {
                // error parsing config file, falling back to defaults
                myLogger.addToLog(F("Error loading settings..."));
                settingsFile.close();

                return false;
        }
        JsonObject& settings=root["settings"];

        strlcpy(ap_ssid,settings["ap"]["ssid"][0] | "",sizeof(ap_ssid));
        myLogger.addToLog(F("ap_ssid: "),ap_ssid);

        strlcpy(ap_pwd,settings["ap"]["pwd"][0] | "",sizeof(ap_pwd));
        myLogger.addToLog(F("ap_pwd: "),ap_pwd);

        strlcpy(sta_ssid,settings["sta"]["ssid"][0] | "",sizeof(sta_ssid));
        myLogger.addToLog(F("sta_ssid: "),sta_ssid);

        strlcpy(sta_pwd,settings["sta"]["pwd"][0] | "",sizeof(sta_pwd));
        myLogger.addToLog(F("sta_pwd: "),sta_pwd);

        sta_dhcp=settings["sta"]["dhcp"][0];
        myLogger.addToLog(F("sta_dhcp: "),String(sta_dhcp));

        strlcpy(sta_ip,settings["sta"]["ip"][0] | "",sizeof(sta_ip));
        myLogger.addToLog(F("sta_ip: "),sta_ip);

        strlcpy(sta_gw,settings["sta"]["gw"][0] | "",sizeof(sta_gw));
        myLogger.addToLog(F("sta_gw: "),sta_gw);

        strlcpy(sta_sn,settings["sta"]["sn"][0] | "",sizeof(sta_sn));
        myLogger.addToLog(F("sta_sn: "),sta_sn);

        ntp_enabled=settings["time"]["ntpen"][0];
        myLogger.addToLog(F("ntp_enabled: "),String(ntp_enabled));

        strlcpy(ntp_server1,settings["time"]["ntps1"][0] | "",sizeof(ntp_server1));
        myLogger.addToLog(F("ntp_server1:"),ntp_server1);

        strlcpy(ntp_server2,settings["time"]["ntps2"][0] | "",sizeof(ntp_server2));
        myLogger.addToLog(F("ntp_server2:"),ntp_server2);

        strlcpy(ntp_server3,settings["time"]["ntps3"][0] | "",sizeof(ntp_server3));
        myLogger.addToLog(F("ntp_server3:"),ntp_server3);

        strlcpy(timezone,settings["time"]["tz"][0] | "",sizeof(timezone));
        myLogger.addToLog(F("timezone:"),timezone);

        mqtt_connect=settings["mqtt"]["connect"][0];
        myLogger.addToLog(F("mqtt_connect: "),String(mqtt_connect));

        char temp_host[64];
        strlcpy(temp_host,settings["mqtt"]["host"][0] | "",sizeof(temp_host));
        myLogger.addToLog(F("mqtt host:"),temp_host);

        int temp_port=settings["mqtt"]["port"][0];
        myLogger.addToLog(F("mqtt port:"),String(temp_port));

        char temp_user[64];
        strlcpy(temp_user,settings["mqtt"]["user"][0] | "",sizeof(temp_user));
        myLogger.addToLog(F("mqtt user:"),temp_user);

        char temp_pass[64];
        strlcpy(temp_pass,settings["mqtt"]["pass"][0] | "",sizeof(temp_pass));
        myLogger.addToLog(F("mqtt pass:"),temp_pass);

        if(mqtt_connect) {
          connectMQTT(temp_host, temp_port, temp_user, temp_pass);
        }

        // parsing seems to have been successful
        myLogger.addToLog(F("Done loading settings."));
        settingsFile.close();

        return true;
}

std::pair<int, std::unique_ptr<char[]>> WebController::decode64(const char* data, size_t length)
{
    // decoded data is smaller than input.
    size_t size = base64_decode_expected_len(length)+1;
    //myLogger.addToLog("Size: ",String(size));
    std::unique_ptr<char[]> buffer(new char[size]);
    if(buffer) {
        //myLogger.addToLog("Starting decode");
        base64_decodestate _state;
        base64_init_decodestate(&_state);
        //myLogger.addToLog("Starting blocks");
        int len = base64_decode_block(data, length, buffer.get(), &_state);
        buffer.get()[len]=0;
        //myLogger.addToLog("Creating String");
        myLogger.addToLog(F("Calculated size: "),String(size));
        myLogger.addToLog(F("Real size: "),String(len));
        //String decoded = String(buffer);
        //myLogger.addToLog("Free heap: ",String(ESP.getFreeHeap()));
        //free(buffer);
        //myLogger.addToLog("freeing");
        return std::make_pair(len, std::move(buffer));
    }
    return std::make_pair(0, nullptr);
}

void WebController::restoreSettings()
{
  if(restoreSettingsFile=="") return;

  myLogger.addToLog(F("Restoring from : "),restoreSettingsFile);
  sendEvent("Restore","{\"type\":\"started\"}");
  File tempFile=SPIFFS.open(restoreSettingsFile,"r");
  restoreSettingsFile="";

  DynamicJsonBuffer jsonBuffer;
  JsonArray& root = jsonBuffer.parseArray(tempFile);

  if(root==JsonArray::invalid()) {
          // error parsing uploaded config file, falling back to defaults
          myLogger.addToLog(F("Error parsing uploaded file."));
          sendEvent("Restore","{\"type\":\"failed\", \"reason\":\"Error parsing uploaded file.\"}");
          tempFile.close();
          return;
  }

  for(int i=0; i<root.size(); i++) {
    JsonObject& file=root[i];
    sendEvent("Restore","{\"type\":\"progress\", \"current\": "+String(i)+", \"total\": "+String(root.size())+"}");

    // iterate over all files in the uploaded data
    if(!(file.containsKey("filepath") && file.containsKey("content"))) {
      sendEvent("Restore","{\"type\":\"warning\", \"reason\":\"Found invalid JSON object with missing key filepath and/or content.\"}");
      continue;
    }

    // check for valid filename
    if(!file["filepath"].is<char*>() || file["filepath"].as<String>().length()>31 || file["filepath"].as<String>()=="") {
      sendEvent("Restore","{\"type\":\"warning\", \"reason\":\"Filepath \""+file["filepath"].as<String>()+"\" invalid.\"}");
      continue;
    }

    sendEvent("Restore","{\"type\":\"filestatus\",\"filepath\":\""+file["filepath"].as<String>()+"\",\"status\":\"started\"}");

    // try to decode file contents
    std::pair<int,std::unique_ptr<char[]>> decoded=decode64(file["content"].as<char*>(),file["content"].as<String>().length());

    if(decoded.first==0 && decoded.second==nullptr) {
      sendEvent("Restore","{\"type\":\"filestatus\",\"filepath\":\""+file["filepath"].as<String>()+"\",\"status\":\"failed\",\"reason\":\"base64 decode failed.\"}");
      continue;
    } else {
      File myFile=SPIFFS.open(file["filepath"].as<String>(),"w");

      if(myFile) {
        myFile.write((uint8_t*) decoded.second.get(),decoded.first);
        sendEvent("Restore","{\"type\":\"filestatus\",\"filepath\":\""+file["filepath"].as<String>()+"\",\"status\":\"success\"}");
        myFile.close();
      } else {
        sendEvent("Restore","{\"type\":\"filestatus\",\"filepath\":\""+file["filepath"].as<String>()+"\",\"status\":\"failed\",\"reason\":\"Error opening file.\"}");
        continue;
      }
    }
  }
  sendEvent("Restore","{\"type\":\"done\"}");

  tempFile.close();
  SPIFFS.remove(restoreSettingsFile);
  restoreSettingsFile="";

  reboot=true;
}

void WebController::addSettingsIncludePath(String path)
{
  settings_includepaths.push_back(path);
}

void WebController::resetSettingsIncludePaths()
{
  settings_includepaths.clear();
}

void WebController::addSettingsBlacklistFilename(String filename)
{
  settings_filesblacklist.push_back(filename);
}

void WebController::resetSettingsBlacklist()
{
  settings_filesblacklist.clear();
  settings_filesblacklist.push_back("/config/webserver.json");
}

void WebController::addServerHandler(const char* uri, WebRequestMethodComposite method, ArRequestHandlerFunction onRequest, bool acceptFileUpload, bool acceptBodyUpload)
{
  AsyncCallbackWebHandler* handler = new AsyncCallbackWebHandler();
  handler->setUri(uri);
  handler->setMethod(method);
  handler->onRequest(onRequest);
  if(acceptFileUpload) {
    handler->onUpload(std::bind(&WebController::handleFileUpload, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5, std::placeholders::_6));
  }
  if(acceptBodyUpload) {
    handler->onBody(std::bind(&WebController::handleRequestBody, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
  }
  server.addHandler(handler);
}

void WebController::addServerHandler(const char* uri, WebRequestMethodComposite method, std::function<void(AsyncWebServerRequest *request, String parameters)> onRequest)
{
  additionalWebHandlers.push_back(WebHandlerContainer(uri,method,onRequest));
}

void WebController::connectMQTT(String host, int port, String user, String pass) //, String certpath)
{
  mqtt_host=host;
  mqtt_port=port;
  mqtt_user=user;
  mqtt_pass=pass;

  reconnectMQTT();

  mqtt_enabled=true;
}

void WebController::reconnectMQTT()
{
  mqtt_wificlient=nullptr;
  mqtt_pubsubclient=nullptr;

  mqtt_wificlient=std::shared_ptr<WiFiClient>(new WiFiClient);
  mqtt_pubsubclient=std::unique_ptr<PubSubClient>(new PubSubClient(mqtt_host.c_str(), mqtt_port, *mqtt_wificlient));

  mqtt_pubsubclient->setCallback(std::bind(&WebController::subscribeCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

  mqtt_pubsubclient->connect((appname+String(ESP.getChipId(),HEX)).c_str(), mqtt_user.c_str(), mqtt_pass.c_str());

  if(mqtt_pubsubclient->connected()) {
    myLogger.addToLog(F("Connected to MQTT"));

    // resubscribe topics
    for(std::map<String, std::vector<std::function<void(String, String)>>>::iterator it=mqtt_subtopics.begin(); it!=mqtt_subtopics.end(); it++) {
      bool result=mqtt_pubsubclient->subscribe(it->first.c_str());
      myLogger.addToLog("Subscribing to "+it->first+": "+String(result));
    }
  } else {
    myLogger.addToLog(F("Connection to MQTT failed"));

    mqtt_pubsubclient=nullptr;
    mqtt_wificlient=nullptr;
  }
}

void WebController::disconnectMQTT()
{
  mqtt_pubsubclient->disconnect();
  myLogger.addToLog(F("MQTT disconnected"));

  mqtt_enabled=false;

  mqtt_host="";
  mqtt_user="";
  mqtt_pass="";

  mqtt_pubsubclient=nullptr;
  mqtt_wificlient=nullptr;
}

bool WebController::publishMQTT(String topic, String value)
{
  if(mqtt_pubsubclient==nullptr || !mqtt_enabled || !mqtt_pubsubclient->connected()) {
    return false;
  }

  return mqtt_pubsubclient->publish(topic.c_str(),value.c_str());
}

unsigned int WebController::subscribeMQTT(String topic, std::function<void(String, String)> onReceive)
{
  if(mqtt_subtopics.count(topic)>0) {
    mqtt_subtopics[topic].push_back(onReceive);
    return mqtt_subtopics[topic].size()-1;
  } else {
    mqtt_subtopics[topic].push_back(onReceive);

    if(mqtt_pubsubclient!=nullptr && mqtt_pubsubclient->connected()) {
      mqtt_pubsubclient->subscribe(topic.c_str());
    }
    return mqtt_subtopics[topic].size()-1;
  }
}

void WebController::unsubscribeMQTT(String topic, unsigned int vectorid)
{
  myLogger.addToLog(F("unsubscribing: "),topic);
  myLogger.addToLog(String(vectorid));
  (mqtt_subtopics[topic])[vectorid]=nullptr;
}

void WebController::subscribeCallback(char *topic, uint8_t *payload, unsigned int length)
{
  char buffer[length+1];
  std::copy(payload, payload + length, buffer);
  buffer[length]='\0';
  String buffer_str=String(buffer);

  if(mqtt_subtopics.count(topic)>0) {
    for(unsigned int i=0; i<mqtt_subtopics[topic].size(); i++) {
      if(mqtt_subtopics[topic][i]!=nullptr) {
        ((mqtt_subtopics[topic])[i])(topic,buffer_str);
      }
    }
  }
}

void WebController::enableOTA()
{
  ota_server=std::unique_ptr<ArduinoOTAClass>(new ArduinoOTAClass());

  ota_server->setHostname(ap_ssid);

  ota_server->onStart([this]() {
    String type;
    if (ota_server->getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      type = "filesystem";
      SPIFFS.end();
    }

    sendEvent("OTA", "{\"status\": \"active\"}");

    myLogger.addToLog(F("Start updating "), type);
  });

  ota_server->onEnd([]() {
    myLogger.addToLog(F("\nEnd"));
  });

  ota_server->onProgress([](unsigned int progress, unsigned int total) {
    myLogger.addToLog(F("Progress: "),String(progress / (total / 100)));
  });

  ota_server->onError([](ota_error_t error) {
    myLogger.addToLogLineStart("Error["+String(error)+"]: ");
    if (error == OTA_AUTH_ERROR) {
      myLogger.addToLogLineEnd(F("Auth Failed"));
    } else if (error == OTA_BEGIN_ERROR) {
      myLogger.addToLogLineEnd(F("Begin Failed"));
    } else if (error == OTA_CONNECT_ERROR) {
      myLogger.addToLogLineEnd(F("Connect Failed"));
    } else if (error == OTA_RECEIVE_ERROR) {
      myLogger.addToLogLineEnd(F("Receive Failed"));
    } else if (error == OTA_END_ERROR) {
      myLogger.addToLogLineEnd(F("End Failed"));
    }
  });

  ota_server->begin();
  sendEvent("OTA", "{\"status\": \"enabled\"}");
  myLogger.addToLog(F("OTA server ready"));
}

void WebController::disableOTA()
{
  ota_server=nullptr;
  sendEvent("OTA", "{\"status\": \"disabled\"}");
}

void WebController::handleEnableOTA(AsyncWebServerRequest * request)
{
  enableOTA();
  request->send(200);
}

void WebController::handleDisableOTA(AsyncWebServerRequest * request)
{
  disableOTA();
  request->send(200);
}

void WebController::handleOTAStatus(AsyncWebServerRequest * request)
{
  request->send(200,"application/json",String("{\"enabled\":")+String(ota_server==nullptr ? "false" : "true")+String("}"));
}

void WebController::handleEnableSerial(AsyncWebServerRequest * request)
{
  myLogger.enableSerialForward(std::bind(&WebController::sendEvent, this, "Serial", std::placeholders::_1, true));

  request->send(200);
}

void WebController::handleDisableSerial(AsyncWebServerRequest * request)
{
  myLogger.disableSerialForward();

  request->send(200);
}

void WebController::handleGetSerialStatus(AsyncWebServerRequest * request)
{
  request->send(200,"application/json",String("{\"enabled\":")+String(myLogger.isSerialEnabled() ? "true" : "false")+String("}"));
}
