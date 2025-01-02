/**
 * @file easyWifi.h
 * @brief Handles Wi-Fi management with captive portal setup, NVS storage.
 * @author Arthur Github: https://github.com/Arthur5492
 */
#ifndef _EASY_WIFI_H_
#define _EASY_WIFI_H_
#pragma once

//? EasyWifi Configs
#define CaptivePortalSSID "Configure_ESP32" //max 63 char
#define CaptivePortalPassword "12345678" //min 8 char, for open use NULL
#define CaptivePortalTimeout 300000 // 5 minutes default (300000)ms

#include <Arduino.h>
#include <FS.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <Preferences.h>
#include <LittleFS.h>

class EasyWifi
{
  public:
    EasyWifi() = default;

    //Core Methods
    void setup(); 
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
    
    // Captive Portal
    AsyncWebServer *_server = nullptr; //Pointer to reduce memory usage
    DNSServer *_dnsServer = nullptr;
    unsigned long _serverStartTime = 0;
    
    //SSID and Password default max length according to WLAN standart
    static constexpr size_t SSID_MAX_LENGTH = 32 + 1;    // +1 to null terminator
    static constexpr size_t PASSWORD_MAX_LENGTH = 64 + 1; // +1 to null terminator

    //?NVS Stored Wi-Fi Credentials
    char _ssidStored[SSID_MAX_LENGTH] = {0}; //Network SSID
    char _passwdStored[PASSWORD_MAX_LENGTH] = {0}; //Network Password
    bool _isEncrypted = false; //Network is encrypted or Open(no password)

    //Wifi Connection setup
    bool _wifiIsReadyToConnect = false;
    bool _wifiConnectionError = false;
    
    Preferences _wifiDataNVS;

    //? Just Constants, ignore
    #define EASYWIFI_ROUTE "/easyWifi"
    static constexpr const char* NVS_NAMESPACE       = "wifiDataNVS";
    static constexpr const char* NVS_KEY_SSID        = "ssid";
    static constexpr const char* NVS_KEY_PASSWD      = "password";
    static constexpr const char* NVS_KEY_ISENCRYPTED = "isEncrypted";
    static constexpr const bool  NVS_READ_WRITE      = false;
    static constexpr const bool  NVS_READ_ONLY       = true;
};

//Singleton
extern EasyWifi easyWifi;

#endif