/**
 * @file easyWifi.h
 * @brief Handles Wi-Fi management with captive portal setup, NVS storage.
 * @author Arthur Github: https://github.com/Arthur5492
 */
#ifndef _EASY_WIFI_H_
#define _EASY_WIFI_H_
#pragma once

#include <Arduino.h>
#include <FS.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <LittleFS.h>



#define AP_DEFAULT_SSID "Configure ESP32"
#define AP_DEFAULT_TIMEOUT 180000 //3 minutes

#define SSID_MAX_LENGTH     32 + 1 // Values according to WLAN standart
#define PASSWORD_MAX_LENGTH 64 + 1 //+1 to null terminator 
    
class EasyWifi
{
  public:
    EasyWifi() = default;

    //Core Methods
    ///@param CaptivePortalSSID SSID of the Captive Portal - Max 33 characters
    ///@param CaptivePortalPassword Password of the Captive Portal, leave empty for open network
    ///@param CaptivePortalTimeout Timeout in milliseconds to wait for a sucefful connection - 3min default
    void setup(const char* CaptivePortalSSID=AP_DEFAULT_SSID, const char* CaptivePortalPassword="", unsigned long CaptivePortalTimeout =AP_DEFAULT_TIMEOUT); 

    void loop(); 
    bool connectWifi();

    //Captive Portal
    void startCaptivePortal(); 
    void logoutCaptivePortal(); 
    void serveStaticRoutes();
    
    // Web Server Controllers
    void wifiScanController(AsyncWebServerRequest *request);
    void SaveWiFiDataController(AsyncWebServerRequest *request);
    void checkWiFiStatusController(AsyncWebServerRequest *request);
    void redirectToIpController(AsyncWebServerRequest *request);
    
    //NVS Functions - (Non-Volatile Storage)
    bool NVS_SaveWifiSettings();
    bool NVS_Clear(); 
    bool NVS_RetrieveWifiData();
    
    //Misc
     void clearWebServerPointers();
    //Getters
    const char* get_ssidStored() { return _ssidStored; };
    const char* get_passwdStored() { return _passwdStored; };
    
  private:
    
    //?User configs
    String _CaptivePortalSSID; //sry for using String
    String _CaptivePortalPassword;
    unsigned long _CaptivePortalTimeout; //5 minutes default(300000)

    // Captive Portal
    AsyncWebServer *_server = nullptr; //Pointer to reduce memory usage
    DNSServer *_dnsServer = nullptr;
    unsigned long _serverStartTime = 0;
    
    //SSID and Password default max length according to WLAN standart


    //?NVS Stored Wi-Fi Credentials
    char _ssidStored[SSID_MAX_LENGTH] = {0}; //Network SSID
    char _passwdStored[PASSWORD_MAX_LENGTH] = {0}; //Network Password
    bool _isProtected = false; //Network is protected or Open(no password)

    //Wifi Connection setup
    bool _wifiIsReadyToConnect = false;
    bool _wifiConnectionError = false;
    
    Preferences _wifiDataNVS;

    //? Just Constants, ignore
    #define EASYWIFI_ROUTE "/easyWifi"
    static constexpr const char* NVS_NAMESPACE       = "wifiDataNVS";
    static constexpr const char* NVS_KEY_SSID        = "ssid";
    static constexpr const char* NVS_KEY_PASSWD      = "password";
    static constexpr const char* NVS_KEY_ISPROTECTED = "isProtected";
    static constexpr const bool  NVS_READ_WRITE      = false;
    static constexpr const bool  NVS_READ_ONLY       = true;
};

//Singleton
extern EasyWifi easyWifi;

#endif