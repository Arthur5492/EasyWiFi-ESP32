// Description: This example shows how to check the stored data in the ESP32's flash memory.
#include <Arduino.h>
#include <easyWifi.h>

void setup()
{
    Serial.begin(115200);
    easyWifi.setup();
    //easyWifi.setup("esp32 config", 12345678, 300000); //You can customize the captive portal
}

void loop()
{
    easyWifi.update();
    
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