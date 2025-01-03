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
#define AP_DEFAULT_TIMEOUT 300000 
#define AP_DEFAULT_PASSWORD ""

//SSID and Password default max length according to WLAN standart
#define SSID_MAX_LENGTH     32
#define PASSWORD_MAX_LENGTH 63

enum class WIFI_STATUS{
  ERROR = -1,
  IDLE,
  READY_TO_CONNECT,
  CONNECTING,
  CONNECTED,
};

enum class SCAN_STATUS{
  NOT_RUNNING,
  READY_TO_SCAN,
  RUNNING,
  FINISHED,
};

class EasyWifi
{
  public:
    EasyWifi() = default;

    //Core Methods
    ///@param CaptivePortalSSID SSID of the Captive Portal - Max 33 characters
    ///@param CaptivePortalPassword Password of the Captive Portal, leave empty for open network
    ///@param CaptivePortalTimeout Timeout in milliseconds to wait for a sucefful connection - 3min default
    void setup(const char* CaptivePortalSSID=AP_DEFAULT_SSID, const char* CaptivePortalPassword=AP_DEFAULT_PASSWORD, unsigned long CaptivePortalTimeout =AP_DEFAULT_TIMEOUT); 

    void loop(); 
    bool connectWifi();
    
    void scanNetworks();

    //Captive Portal
    void startCaptivePortal(); 
    void logoutCaptivePortal();
    void checkCaptivePortalTimeout();
    void serveStaticRoutes();
    String sendJsonNetworks();

    // Web Server Controllers
    void checkWifiScanController(AsyncWebServerRequest *request);
    void SaveWiFiDataController(AsyncWebServerRequest *request);
    void checkWiFiStatusController(AsyncWebServerRequest *request);
    void redirectToIpController(AsyncWebServerRequest *request);
    
    //NVS Functions - (Non-Volatile Storage)
    bool NVS_SaveWifiSettings();
    bool NVS_Clear(); 
    bool NVS_RetrieveWifiData();
    
    //Helpers
    void clearWebServerPointers();

    //Getters
    const char* get_ssidStored() { return _ssidStored; };
    const bool get_isProtected() { return _isProtected; };
    const char* get_passwdStored() { return _passwdStored; };

    SCAN_STATUS get_ScanState() { return _scanStatus; };
    WIFI_STATUS get_WifiState() { return _wifiStatus; };
    
  private:
    
    //?User configs
    String _CaptivePortalSSID; //sry for using String
    String _CaptivePortalPassword;
    unsigned long _CaptivePortalTimeout; //5 minutes default(300000)

    // Captive Portal
    AsyncWebServer *_server = nullptr; //Pointer to reduce memory usage
    DNSServer *_dnsServer = nullptr;
    unsigned long _serverStartTime = 0;
    
    //?NVS Stored Wi-Fi Credentials
    char _ssidStored[SSID_MAX_LENGTH] = {0}; //Network SSID
    char _passwdStored[PASSWORD_MAX_LENGTH] = {0}; //Network Password
    bool _isProtected = false; //Network is protected or Open(no password)
    Preferences _wifiDataNVS;
    
    //Helpers
    int16_t _avaibleNetworks = -1;
    
    SCAN_STATUS _scanStatus = SCAN_STATUS::NOT_RUNNING;
    WIFI_STATUS _wifiStatus = WIFI_STATUS::IDLE;

    

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