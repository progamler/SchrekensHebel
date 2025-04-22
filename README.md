# SchrekensHebel

## Setting up the build environment for the RP2040 rpi pico W

To configure the build environment for the RP2040, follow these steps:

1. Install the Arduino IDE if you haven't already.
2. Open the Arduino IDE and go to `File > Preferences`.
3. In the "Additional Boards Manager URLs" field, add the following URL: `https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json`.
4. Go to `Tools > Board > Boards Manager`, search for "rp2040" and install the "Raspberry Pi RP2040" package.
5. Select the RP2040 board by going to `Tools > Board` and choosing "Raspberry Pi Pico".
6. Install the necessary libraries by going to `Sketch > Include Library > Manage Libraries` and searching for the required libraries such as `ESP8266WebServer`, `WiFi`, `ArduinoOTA`, `time`, and `Adafruit_NeoPixel`.
7. Open the `SchrekensHebel.ino` file in the Arduino IDE.
8. Connect your RP2040 board to your computer via USB.
9. Select the correct port by going to `Tools > Port` and choosing the appropriate port for your RP2040 board.
10. Click the "Upload" button to compile and upload the code to your RP2040 board.

## Handling library dependencies in `SchrekensHebel.ino`

To handle library dependencies in `SchrekensHebel.ino`, follow these steps:

1. Ensure you have the necessary libraries installed in the Arduino IDE. The required libraries are `ESP8266WebServer`, `WiFi`, `ArduinoOTA`, `time`, and `Adafruit_NeoPixel`.
2. Open the Arduino IDE and go to `Sketch > Include Library > Manage Libraries`.
3. Search for each of the required libraries and install them if they are not already installed.
4. Verify that the `#include` statements for these libraries are present at the beginning of the `SchrekensHebel.ino` file:
   * `#include <ESP8266WebServer.h>`
   * `#include <WiFi.h>`
   * `#include <ArduinoOTA.h>`
   * `#include <time.h>`
   * `#include <Adafruit_NeoPixel.h>`
5. Ensure that the correct board is selected in the Arduino IDE by going to `Tools > Board` and choosing "Raspberry Pi Pico".
6. Connect your RP2040 board to your computer via USB and select the correct port by going to `Tools > Port` and choosing the appropriate port for your RP2040 board.
7. Click the "Upload" button to compile and upload the code to your RP2040 board.
