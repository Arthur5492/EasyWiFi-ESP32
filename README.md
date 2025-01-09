# EasyWifi
<p align="center">
<img src="./images/app.png" alt="App">
</p>

**EasyWifi** is a simple, intuitive, and visually appealing Wi-Fi connection manager for ESP32 devices. It focuses on delivering a seamless experience for end users with minimal setup and user-friendly interfaces.


## Features

- **Captive Portal**: You will be automatically redirected to Configure Wi-Fi settings via a web interface, 
- **Persistent Storage**: Supports NVS for saving credentials.
- **Fallback Mode**: Automatically switches to AP mode if no connection is available.
- **AsyncWebServer Integration**: Provides fast and responsive web interfaces.
- **Beatiful Lighweight and customizable UI**: You can config the webpage using LittleFS or EEPROM
---
# Installation
## Using PlatformIO

PlatformIO is an open-source ecosystem for IoT development with a cross-platform build system, library manager, and full support for Espressif ESP32 development. It works on popular host OS: Mac OS X, Windows, Linux 32/64, Linux ARM (like Raspberry Pi, BeagleBone, CubieBoard).

### Install PlatformIO Extension for VSCode

1. Install [Visual Studio Code](https://code.visualstudio.com/Download) if not already installed.
2. Install the [PlatformIO IDE extension](https://platformio.org/install/ide?install=vscode):

### Create New Project

1. Create new project using "PlatformIO Home > New Project", Follow the [`documentation`](https://docs.platformio.org/en/latest/core/quickstart.html)

### Add "EasyWifi" Library to Project
Add **EasyWifi** to your project using the [`platformio.ini`](https://docs.platformio.org/en/latest/projectconf/index.html) configuration file and the [`lib_deps`](https://docs.platformio.org/en/latest/projectconf/sections/env/options/library/index.html#lib-deps) option:

```ini
[env:myboard]
platform = espressif32
board = esp32dev
framework = arduino

#using latest stable version
lib_deps = tuizins/EasyWifi
# Using the latest development version
lib_deps = https://github.com/Arthur5492/EasyWiFi-ESP32.git
```

### Set up your main.cpp, lets get an example:

```
#include <Arduino.h>
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
```

# Contributing

This project is still under development. If you find any bugs or have ideas for improvements, please create an issue or submit a pull request. Thank you!






