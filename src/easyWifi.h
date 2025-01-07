/*
 EasyWifi.h Handles Wi-Fi management for ESP32 devices
 Copyright (c) 2025 Arthur Fernandes. All right reserved.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include <Preferences.h>

/* Uncomment the #define to use LittleFS instead of PROGMEM, LittleFS is better when the compile time become bigger
! Don't forget to copy /data folder in EasyWifi directory to your root folder
! Don't forget to add: board_build.filesystem = littlefs inside platformio.ini
? For more information check: https://docs.platformio.org/en/latest/platforms/espressif32.html#uploading-files-to-file-system
*/

//#define EASYWIFI_LITTLEFS 

#ifdef EASYWIFI_LITTLEFS 
  #include <LittleFS.h>
#endif

//TODO: Implement private debug method
//TODO: Implement embedded frontend, maybe with platformio.ini: board_build.embed_txtfiles or directly with EEPROM
//TODO: Implement multi wifi stored, perhaps with a json in littlefs(tip: setting up scanNetworks first and then check if any ssid stored matches with networks discovered)

#define AP_DEFAULT_SSID "Configure ESP32"
#define AP_DEFAULT_PASSWORD ""
#define AP_DEFAULT_TIMEOUT 300000 //5 minutes default(300000)

//SSID and Password default max length according to WLAN standart
#define SSID_MAX_LENGTH     32
#define PASSWORD_MAX_LENGTH 64

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
    bool connectWifi(bool disableautoreconnect);
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
    
    //?User configs
    String _CaptivePortalSSID = AP_DEFAULT_SSID; //sry for using String
    String _CaptivePortalPassword = AP_DEFAULT_PASSWORD;
    unsigned long _CaptivePortalTimeout = AP_DEFAULT_TIMEOUT;

    // Captive Portal
    AsyncWebServer *_server = nullptr; //Pointer to reduce memory usage
    DNSServer *_dnsServer = nullptr;
    unsigned long _serverStartTime = 0;
    bool isCaptivePortalEnabled = false;
    
    //?NVS (Stores Wi-Fi Credentials)
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

//Singleton
extern EasyWifi easyWifi;
