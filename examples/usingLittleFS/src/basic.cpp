/*
* This is a basic example of how to use the easyWifi library with LittleFS.
!!! Remember to set up the /data/easyWifi directory!!!
*/

#define EASYWIFI_LITTLEFS  //this is needed
#include <Arduino.h>
#include "easyWifi.h"

void setup() {
  Serial.begin(115200);

  easyWifi.setup();
}

void loop() {
  easyWifi.update(); //Just like that
  //.... Your code here
  delay(10);
}

