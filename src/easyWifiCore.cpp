//easyWifiCore.cpp

#include <easyWifi.h>

//Singleton
EasyWifi easyWifi;

void EasyWifi::setup(const char* CaptivePortalSSID, const char* CaptivePortalPassword, unsigned long CaptivePortalTimeout)
{
  _CaptivePortalSSID = CaptivePortalSSID;
  _CaptivePortalPassword = CaptivePortalPassword;
  _CaptivePortalTimeout = CaptivePortalTimeout;

  if(NVS_RetrieveWifiData())
    connectWifi();
  else
    startCaptivePortal();
}

/// @return True if connection was successful, false if Failed
bool EasyWifi::connectWifi()
{
  if(_isProtected)
    WiFi.begin(_ssidStored,_passwdStored);
  else
    WiFi.begin(_ssidStored);

  Serial.printf("Trying to connect to Wifi:%s\n",_ssidStored);

  //While loop inside this function
  if(WiFi.waitForConnectResult(8000) != WL_CONNECTED)
  {
     Serial.printf("Failed connection to: %s, continuing to soft AP.\n",_ssidStored);
     _wifiConnectionError = true;
    startCaptivePortal();
    return false;
  }

  Serial.printf("Connected to: %s \n", _ssidStored);
  return true;
}

void EasyWifi::loop()
{ 
  if(!_server)
    return;

  //Captive Portal Timeout Verification
  if(millis() - _serverStartTime >= _CaptivePortalTimeout)
  {
    Serial.println("Captive Portal Timeout, shutting down...");
    logoutCaptivePortal();
    return;
  } 

  if(_dnsServer && WiFi.softAPgetStationNum() > 0) //If there are clients connected to the AP
    _dnsServer->processNextRequest(); 

  if(_wifiIsReadyToConnect)
  {
    _wifiIsReadyToConnect = false;

    bool wifiGotConnected = connectWifi();
    if(wifiGotConnected)
      NVS_SaveWifiSettings();
    else
      _wifiConnectionError = true;
  }
}

bool EasyWifi::NVS_RetrieveWifiData()
{ 
  _wifiDataNVS.begin(NVS_NAMESPACE,NVS_READ_ONLY);
  
  if(!(_wifiDataNVS.isKey(NVS_KEY_SSID))) 
  {
    Serial.printf("No SSID found on NVS memory\n");
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
    Serial.println("Error: SSID retrieval failed or empty.");
    _wifiDataNVS.end();
    return false;
  }

  //Check if Password is Encrypted and correctly retrieved
  if(_isProtected && passwdLength <= 0)
  {
    Serial.println("Error: Password retrieval failed or empty for encrypted network.");
    _wifiDataNVS.end();
    return false;
  }
  
  _wifiDataNVS.end();
  Serial.printf("SSID Found: %s\n",_ssidStored); 
  return true;
}

bool EasyWifi::NVS_Clear() {
  
  _wifiDataNVS.begin(NVS_NAMESPACE, NVS_READ_WRITE);

  if(_wifiDataNVS.clear() == false)
  {
    Serial.println("Error while cleaning NVS");
    _wifiDataNVS.end();
    return false;  
  }

  Serial.println("NVS Cleared");
  _wifiDataNVS.end();
  return true; 
}

bool EasyWifi::NVS_SaveWifiSettings()
{
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
    Serial.println("Error saving data in NVS");
    return false;
  }


  Serial.printf("SSID: %s saved on NVS\n",_ssidStored);
  _wifiDataNVS.end();
  return true;
}

void EasyWifi::clearWebServerPointers()
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