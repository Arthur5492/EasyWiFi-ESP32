#include "easyWifi.h"

void EasyWifi::startCaptivePortal()
{
  if(_server)
    return;

  WiFi.mode(WIFI_AP_STA); //AP for cap portal STA for scanNetworks
  WiFi.softAP(CaptivePortalSSID, CaptivePortalPassword);

  _server = new AsyncWebServer(80);
  _dnsServer = new DNSServer();
  _dnsServer->start(53, "*", WiFi.softAPIP());

  _server->onNotFound([this](AsyncWebServerRequest *request) {    
      redirectToIpController(request);
  });

  serveStaticRoutes();

  // Route to scan and send wi-fi networks
  _server->on("/scan", HTTP_GET, [this](AsyncWebServerRequest *request) {
    wifiScanController(request); 
  });

  _server->on("/save", HTTP_POST, [this](AsyncWebServerRequest *request) {
    SaveWiFiDataController(request);
  });

  //After client try to connect to wifi this route will be called to check the status
  _server->on("/wifi-status",HTTP_GET,[this](AsyncWebServerRequest *request){
    checkWiFiStatusController(request);

  });

  _server->begin();
  _serverStartTime = millis();
  Serial.println("Captive Portal initializated\n");
}

void EasyWifi::logoutCaptivePortal()
{  
  if(!_server)
    return;

  if(_server)
    _server->end();

  delay(2000); //Wait for server to end, dns to process last requests and user be able to see the connected message

  if(_dnsServer)
    _dnsServer->stop();
  
  clearWebServerPointers();

  WiFi.softAPdisconnect(true);
  Serial.printf("Captive Portal ended\n");
}


//First request will return 0 results unless you start scan from somewhere else (loop/setup)
//Do not request more often than 3-5 seconds
void EasyWifi::wifiScanController(AsyncWebServerRequest *request)
{
  // Obtém o status do scan
  int16_t status_wifiScan = WiFi.scanComplete();

  //JSON START - I didn't use ArduinoJson to reduce memory usage
  String json = "[";

  if (status_wifiScan == -2){ // -2 = scan not started or failed 
    WiFi.scanNetworks(true);
    Serial.println("Scan started");
  }

  else if (status_wifiScan > 0) // Networks found
  { 
    for (int i = 0; i < status_wifiScan; i++)
    {
      if (i) json += ","; // Dot if not the first element
      json += "{";
      json += "\"ssid\":\"" + WiFi.SSID(i) + "\"";
      json += ",\"rssi\":" + String(WiFi.RSSI(i));
      json += ",\"isProtected\":" + String(WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
      json += "}";
    }
    WiFi.scanDelete();
    Serial.printf("Scan Found %d networks\n", status_wifiScan);
  }
  json += "]"; //Finished JSON

  request->send(200, "application/json", json);
  json.clear();
}


void EasyWifi::SaveWiFiDataController(AsyncWebServerRequest *request)
{
  //Check if the form was filled
  if(!request->hasArg("ssid") || !request->hasArg("password") || !request->hasArg("isProtected")) {
    request->send(400, "text/plain", "Missing parameters"); 
    return;
  }
  
  //Get all data from the form
  String ssid = request->arg("ssid");
  String passwd = request->arg("password");
  _isEncrypted = request->hasArg("isProtected");

  //Check if the SSID is Empty
  if(ssid.length() <= 0)
  { 
    request->send(400, "text/plain", "Invalid SSID..."); 
    return;
  }

  if(_isEncrypted && passwd.length() < 8)
  {
    request->send(400, "text/plain", "Password must have at least 8 characters");
    return;
  }

  ssid.toCharArray(_ssidStored, SSID_MAX_LENGTH);
  passwd.toCharArray(_passwdStored, PASSWORD_MAX_LENGTH);
  
  Serial.printf("ssid found: %s\n", _ssidStored);
  
  request->send(200, "text/plain", "Data received, trying to connect to Wi-Fi...");


  _wifiIsReadyToConnect = true; //Value is checked on loop();
  return;
}

void EasyWifi::checkWiFiStatusController(AsyncWebServerRequest *request)
{
  if(_wifiConnectionError){
    request->send(400, "text/plain", "Error connecting to Wi-Fi");
    _wifiConnectionError = false;
    return;
  }
    
  if(WiFi.isConnected())
  {
    request->send(200, "text/plain", "Connected to Wifi! \n Shutting down Captive Portal...");
    logoutCaptivePortal();
  }
  else
    request->send(202, "text/plain", "Trying Connection..."); //Indicates that the request is still being processed
  
  return;
}

/* 
*   Captive portal is set to a different route depending of the dispositive, 
*   so I used the onNotFound method to redirect to my Ip route instead.
*   I didn't use asyncWebServer captive portal example just to simplify the amount of code, but it does the same thing 
*/
void EasyWifi::redirectToIpController(AsyncWebServerRequest *request)
{
  String apIPString = WiFi.softAPIP().toString();
  String correctUrl = "http://" + apIPString + EASYWIFI_ROUTE;

  if ( !(request->host().equals(apIPString)) ) //Check if user isn't at Ip url
    request->redirect(correctUrl);
}

void EasyWifi::serveStaticRoutes()
{
  if(!LittleFS.exists("/"))
  {
    Serial.println("LittleFS not initiated, initializing...");
    LittleFS.begin(true);
    return;
  }
  // All static routes are served from the /easyWifi folder
  _server->serveStatic("/easyWifi", LittleFS , "/easyWifi/")
        .setDefaultFile("index.htm");
} 