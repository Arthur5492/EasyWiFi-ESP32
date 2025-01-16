//easyWifiCore.cpp

#include <easyWifi.h>

using namespace EASYWIFI;

//Singleton
EasyWifi easyWifi;

void EasyWifi::setup(const char* ssid, const char* passwd, unsigned long timeout)
{
  if(ssid!=nullptr)   _CaptivePortalSSID = ssid;
  if(passwd!=nullptr) _CaptivePortalPassword = passwd;
  if(timeout!=0)      _CaptivePortalTimeout = timeout;
  
  if(NVS_RetrieveWifiData())
    connectWifi();
  else 
    startCaptivePortal();
}

/// @brief Check all events, including the state machine 
void EasyWifi::update()
{ 
  if(!isCaptivePortalEnabled)
    return;

  checkCaptivePortalTimeout();
  
  if(_dnsServer) 
    _dnsServer->processNextRequest(); 

  if(_wifiStatus == WIFI_STATUS::READY_TO_CONNECT)
    connectWifi();

  if(_scanStatus == SCAN_STATUS::READY_TO_SCAN)
    scanNetworks();
}

/// @return True if connection was successful, false if Failed
bool EasyWifi::connectWifi()
{
  ESP_LOGV(APP, "Trying Connection to %s with password: %s", _ssidStored, _passwdStored);

  _wifiStatus = WIFI_STATUS::CONNECTING;
  WiFi.setAutoReconnect(false); // avoid reconnecting to network if WiFi conn fails, will be re-enabled after connection

  if(_isProtected)
    WiFi.begin(_ssidStored,_passwdStored);
  else
    WiFi.begin(_ssidStored);

  //While loop inside waitForConnectResult, with a timeout parameter
  if(WiFi.waitForConnectResult(8000) != WL_CONNECTED)
  {
    _wifiStatus = WIFI_STATUS::ERROR;  
    ESP_LOGE(APP,"Failed to connect to Wifi:%s\n",_ssidStored);

    startCaptivePortal();
    return false;
  }

  ESP_LOGI(APP,"Connected to: %s\n", _ssidStored);

  _wifiStatus = WIFI_STATUS::CONNECTED;
  NVS_SaveWifiSettings();  
  WiFi.setAutoReconnect(true); //Re-enable auto reconnect by default
  return true;
}

//Blocking function, it haves state machine to avoid blocking the server
void EasyWifi::scanNetworks()
{
  ESP_LOGV(APP, "Scanning Networks");

  _scanStatus = SCAN_STATUS::RUNNING;
  _avaibleNetworks = WiFi.scanNetworks();

  ESP_LOGV(APP,"Scan found %d networks",_avaibleNetworks);
  _scanStatus = SCAN_STATUS::FINISHED;
}

bool EasyWifi::NVS_RetrieveWifiData()
{ 
  ESP_LOGV(APP, "Retrieving WiFi data from NVS");

  _wifiDataNVS.begin(NVS_NAMESPACE,NVS_READ_ONLY);
  
  if(!(_wifiDataNVS.isKey(NVS_KEY_SSID))) 
  {
    ESP_LOGI(APP,"No SSID found on NVS memory");
    _wifiDataNVS.end();
    return false;
  }

  //Retrieve Data 
  size_t ssidLength = _wifiDataNVS.getString(NVS_KEY_SSID, _ssidStored, SSID_MAX_LENGTH);
  size_t passwdLength = _wifiDataNVS.getString(NVS_KEY_PASSWD, _passwdStored, PASSWORD_MAX_LENGTH);
  _isProtected = _wifiDataNVS.getBool(NVS_KEY_ISPROTECTED);

  //Check if SSID Exists
  if(ssidLength <= 0)
  {
    ESP_LOGE(APP,"Error: SSID retrieval failed or empty.");
    _wifiDataNVS.end();
    return false;
  }

  //Check if Password is Encrypted and correctly retrieved
  if(_isProtected && passwdLength <= 0)
  {
    ESP_LOGE(APP,"Error: Password retrieval failed or empty for encrypted network.");
    _wifiDataNVS.end();
    return false;
  }
  
  _wifiDataNVS.end();
  ESP_LOGI(APP,"SSID Found: %s",_ssidStored);
  return true;
}

bool EasyWifi::NVS_Clear() 
{
  ESP_LOGI(APP, "Clearing NVS Memory");
  _wifiDataNVS.begin(NVS_NAMESPACE, NVS_READ_WRITE);

  if(_wifiDataNVS.clear() == false)
  {
    ESP_LOGE(APP,"Error while cleaning NVS");
    _wifiDataNVS.end();
    return false;  
  }

  ESP_LOGI(APP,"NVS Cleared");
  _wifiDataNVS.end();
  return true; 
}

bool EasyWifi::NVS_SaveWifiSettings()
{
  ESP_LOGI(APP, "Saving WiFi data on NVS");
  _wifiDataNVS.begin(NVS_NAMESPACE,NVS_READ_WRITE);

  _wifiDataNVS.clear();

  //NVS returns 0 if error occurs
  bool ssidCheck = _wifiDataNVS.putString(NVS_KEY_SSID, _ssidStored) != 0;
  bool isProtectedCheck = _wifiDataNVS.putBool(NVS_KEY_ISPROTECTED, _isProtected);
  bool passwordCheck = true;

  if(_isProtected && isProtectedCheck) //If network is protected and suceffuly saved on nvs
  {
    _wifiDataNVS.putString(NVS_KEY_PASSWD, _passwdStored);
    passwordCheck = _wifiDataNVS.putString(NVS_KEY_PASSWD, _passwdStored) != 0;
  }

  if(!ssidCheck || !isProtectedCheck || !passwordCheck)
  {
    ESP_LOGE(APP,"Error saving data in NVS");
    return false;
  }


  ESP_LOGI(APP,"SSID: %s saved on NVS",_ssidStored);
  _wifiDataNVS.end();
  return true;
}