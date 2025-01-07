/*
* This is a basic example of how to use the easyWifi library in platformio.
! On Arduino IDE you need to manually run the gen_frontend_header.py once to generate the frontend.h file
*/

#include <easyWifi.h>

// Optional: Define a custom timeout for the Captive Portal (in milliseconds)
#define SHUTDOWN_TIMEOUT 180000 //You can define the timeout for the captive portal to shutdown

void setup() {
  Serial.begin(115200);

  // Example 1: Use default Captive Portal settings 
  easyWifi.setup();

  // Example 2: Customize all at once
  // easyWifi.setup("MyESP32", "12345678", SHUTDOWN_TIMEOUT);
}

void loop() {
  easyWifi.update(); //Just like that
  //.... Your code here
  delay(10);
}

