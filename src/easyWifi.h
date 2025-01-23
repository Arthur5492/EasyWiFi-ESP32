#pragma once

#include <Arduino.h>
#include <WiFi.h> //WiFi Library
#include <ESPAsyncWebServer.h> //Async Web Server
#include <DNSServer.h> //Local DNS Server used for redirecting all requests to the configuration portal
#include <Preferences.h> //To store Wi-Fi Credentials
#include <esp_log.h> //For logging

#ifdef EASYWIFI_LITTLEFS 
  #include <LittleFS.h>
#endif

//TODO: Simplify the data stored in NVS, removing the isProtected
//TODO: Implement multi wifi stored, perhaps with a wifiParam struct or a json in littlefs(tip: setting up scanNetworks first and then check if any ssid stored matches with networks discovered)

#define AP_DEFAULT_SSID "Configure ESP32"
#define AP_DEFAULT_PASSWORD ""
#define AP_DEFAULT_TIMEOUT 300000 //5 minutes default(300000)

//SSID and Password default max length according to WLAN standart
#define SSID_MAX_LENGTH     32
#define PASSWORD_MAX_LENGTH 64


namespace EASYWIFI{

static const char* APP = "EasyWifi";

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
    ///@param ssid SSID of the Captive Portal - Max 32 characters
    ///@param passwd Password of the Captive Portal, leave empty for open network
    ///@param timeout Timeout in ms for captive portal
    void setup(const char* ssid=nullptr, const char* passwd=nullptr, unsigned long timeout=0); 

    void update(); //Check all events
    bool connectWifi();
    void scanNetworks();

    //Captive Portal
    void startCaptivePortal(); 
    void logoutCaptivePortal();
    void checkCaptivePortalTimeout();
    
    String sendJsonNetworks();

    //Serve AsyncWebServer Routes
    void serveScanRoutes();
    void serveWifiRoutes();
    void serveStaticRoutes();
    
    // Web Server Controllers
    void checkScanController(AsyncWebServerRequest *request);
    void SaveWiFiDataController(AsyncWebServerRequest *request);
    void checkWiFiStatusController(AsyncWebServerRequest *request);
    void redirectToIpController(AsyncWebServerRequest *request);
    
    //NVS Functions - (Non-Volatile Storage)
    bool NVS_SaveWifiSettings();
    bool NVS_Clear(); 
    bool NVS_RetrieveWifiData();
    
    //Helpers
    void freePointers();

    //Getters
    const char* get_ssidStored() { return _ssidStored; };
    const bool get_isProtected() { return _isProtected; };
    const char* get_passwdStored() { return _passwdStored; };

    SCAN_STATUS get_ScanState() { return _scanStatus; };
    WIFI_STATUS get_WifiState() { return _wifiStatus; };
    
  private:
    
    // User configs
    String _CaptivePortalSSID = AP_DEFAULT_SSID; //sry for using String
    String _CaptivePortalPassword = AP_DEFAULT_PASSWORD;
    unsigned long _CaptivePortalTimeout = AP_DEFAULT_TIMEOUT;

    // Captive Portal
    AsyncWebServer *_server = nullptr; //Pointer to reduce memory usage
    DNSServer *_dnsServer = nullptr;
    unsigned long _serverStartTime = 0;
    bool isCaptivePortalEnabled = false;
    
    // NVS (Stores Wi-Fi Credentials)
    Preferences _wifiDataNVS;
    char _ssidStored[SSID_MAX_LENGTH] = {0}; //Network SSID
    char _passwdStored[PASSWORD_MAX_LENGTH] = {0}; //Network Password
    bool _isProtected = false; //Network is protected or Open(no password)
    
    //Helpers
    int16_t _avaibleNetworks = -1;
    SCAN_STATUS _scanStatus = SCAN_STATUS::NOT_RUNNING;
    WIFI_STATUS _wifiStatus = WIFI_STATUS::IDLE;

    //? Just Constants, ignore them
    static constexpr const char* NVS_NAMESPACE       = "wifiDataNVS";
    static constexpr const char* NVS_KEY_SSID        = "ssid";
    static constexpr const char* NVS_KEY_PASSWD      = "password";
    static constexpr const char* NVS_KEY_ISPROTECTED = "isProtected";
    static constexpr const bool  NVS_READ_WRITE      = false;
    static constexpr const bool  NVS_READ_ONLY       = true;
};

};

//Singleton
extern EASYWIFI::EasyWifi easyWifi;
