/*
* This example shows how to set up LittleFS in the EasyWifi library.
! Don't Forget to include the /data/easyWifi folder in your root folder
? You can copy the data folder in my repo https://github.com/Arthur5492/EasyWiFiPortal-ESP32/
? paste in the root of your platformio project, Will not work for SPIFFS
*
! Don't Forget to change your FileSystem to LittleFS 
*Please read the docs:
*Platformio: https://docs.platformio.org/en/latest/platforms/espressif32.html#uploading-files-to-file-system
*
*Arduino IDE: read littleFS Installation and Arduino ESP32 LittleFS filesystem upload tool 
*https://github.com/lorol/LITTLEFS
*/

#define EASYWIFI_LITTLEFS //This is needed
#include <easyWifi.h>

void setup()
{
  easyWifi.setup(); //Same as always
  //easyWifi.setup("esp32 config", 12345678, 300000); //You can customize the captive portal
}

void loop()
{
  easyWifi.update();
  delay(10);
}