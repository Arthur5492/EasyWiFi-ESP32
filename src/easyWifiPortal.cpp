#include "easyWifi.h"

using namespace EASYWIFI;

#ifndef EASYWIFI_LITTLEFS //Include frontend that will be stored in RAM if not using LittleFS
  #include "frontend.h"
#endif

void EasyWifi::startCaptivePortal()
{
  if(_isCaptivePortalEnabled) //If server is already running
    return;

  ESP_LOGI(APP, "Starting Captive Portal");

  WiFi.mode(WIFI_AP_STA); //AP for cap portal STA for scanNetworks - Redundant but better explicited than not

  scanNetworks(); //Scan networks to show on first captive portal opening
  
  WiFi.softAP(_CaptivePortalSSID, _CaptivePortalPassword);
  
  _server = new AsyncWebServer(80);
  _dnsServer = new DNSServer();
  _dnsServer->start(53, "*", WiFi.softAPIP());

  serveScanRoutes();
  serveWifiRoutes();
  
  
  _server->onNotFound([this](AsyncWebServerRequest *request) {
    redirectToIpController(request);
  });
  
  serveStaticRoutes();

  _server->begin();
  _isCaptivePortalEnabled = true;
  _serverStartTime = millis();
  ESP_LOGI(APP,"Captive Portal initializated at: http://%s\n",WiFi.softAPIP().toString().c_str());
}

void EasyWifi::serveScanRoutes()
{
  _server->on("/start-scan", HTTP_GET, [this](AsyncWebServerRequest *request)
  {
    ESP_LOGV(APP,"Scan Requested");
    _scanStatus = SCAN_STATUS::READY_TO_SCAN;
    request->send(200,"text/plain","WiFi Scan Started");
  });

  //After client try to scan the network this route will be called to check the status
  _server->on("/scan-status", HTTP_GET, [this](AsyncWebServerRequest *request){
    checkScanController(request);
  });
}

void EasyWifi::serveWifiRoutes()
{
  _server->on("/start-wifi", HTTP_POST, [this](AsyncWebServerRequest *request) {
    ESP_LOGV(APP,"WiFi Connection Requested from route");
    SaveWiFiDataController(request);
  });

  //After client try to connect to wifi this route will be called to check the status
  _server->on("/wifi-status",HTTP_GET,[this](AsyncWebServerRequest *request){
    checkWiFiStatusController(request);
  });
}

//Do not request more often than 3-5 seconds
void EasyWifi::checkScanController(AsyncWebServerRequest *request)
{
  switch(_scanStatus)
  {
    case SCAN_STATUS::NOT_RUNNING: //If Scan is not running, we should change the variable to start the scan
      _scanStatus = SCAN_STATUS::READY_TO_SCAN; //Variable is watched on loop
      request->send(202, "text/plain", "Scan will start soon...");
    break;

    case SCAN_STATUS::RUNNING:
      request->send(202, "text/plain", "Scanning...");
    break;

    case SCAN_STATUS::READY_TO_SCAN:
      request->send(202, "text/plain", "Scan will start soon...");
    break;

    case SCAN_STATUS::FINISHED: //That's what we want
      if(_avaibleNetworks > 0)
        request->send(200, "application/json", sendJsonNetworks());
      else
        ESP_LOGE(APP,"Error, no networks found to process Json, _avaibleNetworks: %d\n",_avaibleNetworks);

      _scanStatus = SCAN_STATUS::NOT_RUNNING;
      _avaibleNetworks = -1; //Reset variable
    break;

    default:
      ESP_LOGE(APP,"No matching case in checkScanController");
      request->send(500, "text/plain", "Error while scanning networks");
    break;
  };
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
  _isProtected = request->arg("isProtected") == "1"; //"0" = Open Network, "1" = Encrypted Network

  //Check if the SSID is Empty
  if(ssid.length() <= 0)
  {
    request->send(400, "text/plain", "Invalid SSID...");
    return;
  }

  if(_isProtected && passwd.length() < 8)
  {
    request->send(400, "text/plain", "Password must have at least 8 characters");
    return;
  }

  ssid.toCharArray(_ssidStored, SSID_MAX_LENGTH);
  passwd.toCharArray(_passwdStored, PASSWORD_MAX_LENGTH);

  ESP_LOGV(APP,"ssid found in : %s\n", _ssidStored);

  request->send(200, "text/plain", "Data received, trying to connect to Wi-Fi...");


  _wifiStatus = WIFI_STATUS::READY_TO_CONNECT; //Value is checked on loop();
  return;
}

void EasyWifi::checkWiFiStatusController(AsyncWebServerRequest *request)
{
  switch(_wifiStatus)
  {
    case WIFI_STATUS::CONNECTING:
      request->send(202, "text/plain", "Trying Connection...");
    break;

    case WIFI_STATUS::CONNECTED:
      request->send(200, "text/plain", "Connected to Wifi! \n You are free to go ;)");
    break;

    case WIFI_STATUS::ERROR:
      request->send(500, "text/plain", "Error connecting to Wi-Fi");
      _wifiStatus = WIFI_STATUS::IDLE; //Reset status
    break;
  }
}

/*
*   Captive portal is set to a different route depending of the dispositive,
*   so I used the onNotFound method to redirect to my Ip route instead.
*   I didn't use asyncWebServer captive portal example just to simplify the amount of code, but it does the same thing
*/
void EasyWifi::redirectToIpController(AsyncWebServerRequest *request)
{
  String apIPString = "http://" + WiFi.softAPIP().toString();

  if ( !(request->host().equals(apIPString)) ) //Check if user isn't at Ip url
    request->redirect(apIPString);
}

void EasyWifi::serveStaticRoutes()
{
#ifdef EASYWIFI_LITTLEFS
  if(!LittleFS.begin() && LittleFS.open("/easyWifi", "r")){
    ESP_LOGE(APP,"LittleFS Failed, please check if the /data/easyWifi folder exists");
    return;
  }

  // All static routes are served from /easyWifi folder
  _server->serveStatic("/", LittleFS , "/easyWifi")
        .setDefaultFile("index.htm");
#else
  _server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(
      200, "text/html",index_htm_gz,index_htm_gz_len);

    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  _server->on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(
      200, "text/css", style_css_gz,style_css_gz_len);

    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

  _server->on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncWebServerResponse *response = request->beginResponse_P(
      200, "text/javascript", script_js_gz,script_js_gz_len);
      
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });  
#endif  
}

String EasyWifi::sendJsonNetworks()
{
  //JSON START - I didn't use ArduinoJson to reduce memory usage
  String json;
  json.reserve(100 * _avaibleNetworks); //average size of json structure(100 bytes) * number of networks

  json += "[";
    for (int i = 0; i < _avaibleNetworks; i++)
    {
      if (i) json += ","; // Comma if not the first element
      json += "{";
      json += "\"ssid\":\"" + WiFi.SSID(i) + "\"";
      json += ",\"rssi\":" + String(WiFi.RSSI(i));
      json += ",\"isProtected\":" + String(WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
      json += "}";
    }
  json += "]"; //Finished JSON

  return json;
}

void EasyWifi::logoutCaptivePortal()
{
  _isCaptivePortalEnabled = false;

  if(_server)
    _server->end();
  
  if(_dnsServer)
    _dnsServer->stop();
  
  delay(2000); //Wait for server to end, dns to process last requests and user be able to see the connected message

  freePointers();
  WiFi.mode(WIFI_STA); //Back to station mode
  ESP_LOGI(APP,"Captive Portal ended\n");
}

void EasyWifi::freePointers()
{
  if(_dnsServer)
  {
    delete _dnsServer;
    _dnsServer = nullptr;
  }

  if(_server)
  { 
    delete _server;
    _server = nullptr;
  }
}

void EasyWifi::checkCaptivePortalTimeout()
{
  if(millis() - _serverStartTime >= _CaptivePortalTimeout)
  {
    ESP_LOGI(APP,"Captive Portal Timeout, shutting down...");
    logoutCaptivePortal();
    return;
  } 
}