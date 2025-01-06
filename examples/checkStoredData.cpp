#include <Arduino>
#include "easyWifi.h"

void setup()
{
    Serial.begin(115200);
    Serial.println("Starting EasyWifi...");
    easyWifi.setup();
}

void loop()
{
    easyWifi.loop();
    
    if(WiFi.isConnected())
    {
      String ssid = easyWiFi.get_ssidStored(); //SSID
      bool isProtected = easyWiFi.get_isProtected();//0 if network is open, 1 if it's Encrypted
      Serial.printf("Connected to Wi-Fi network: %s,\n", ssid.c_str());

      if (isProtected) 
      {
        String passwd = easyWifi.get_passwdStored(); // Senha da rede
        Serial.printf("Password: %s\n", passwd.c_str());
      } 
      else
        Serial.println("This is an open network (no password).\n");
    }
  delay(10000); //10 seconds
}