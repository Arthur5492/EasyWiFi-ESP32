/*
* This is a basic example of how to use the easyWifi library in platformio.
* I didn't set up to Arduino IDE, but it should be similar.
*/

//!!! Remember to set up the /data/easyWifi directory!!!

#include <Arduino.h>
#include "easyWifi.h"

// Optional: Define a custom timeout for the Captive Portal (in milliseconds)
#define SHUTDOWN_TIMEOUT 180000 //You can define the timeout for the captive portal to shutdown

void setup() {
  Serial.begin(115200);

  // Example 1: Use default Captive Portal settings 
  easyWifi.setup();

  // Example 2: Customize all at once
  // easyWifi.setup("MyESP32", "mypassword", SHUTDOWN_TIMEOUT);
}

void loop() {
  easyWifi.loop(); //Just like that
  //.... Your code here
  delay(10);
}

