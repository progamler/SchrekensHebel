#include <ESP8266WebServer.h>
#include <HTTPServer.h>
#include <HTTP_Method.h>
#include <Uri.h>
#include <WebServer.h>
#include <WebServerSecure.h>
#include <WebServerTemplate.h>
#include <WiFi.h>
#include <SimpleMDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <time.h>
#include <sys/time.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>  // Required for 16 MHz Adafruit Trinket
#endif

#define LAMPPIN 5 // On Trinket or Gemma, suggest changing this to 1
#define CASEPIN 4
#define LNUMPIXELS 7  // Popular NeoPixel ring size
#define CNUMPIXELS 4

#ifndef STASSID
#define STASSID "JFZ"
#define STAPSK "clubmate"
#define STAMAC "d8:3a:dd:21:ca:0d"
#endif


const char* ssid = STASSID;
const char* password = STAPSK;

bool open = false;
long int uhrzeit;
ESP8266WebServer server(80);
Adafruit_NeoPixel LAMPpixels(LNUMPIXELS, LAMPPIN, NEO_GRBW + NEO_KHZ800);
Adafruit_NeoPixel CASEpixels(CNUMPIXELS, CASEPIN, NEO_GRBW + NEO_KHZ800);

String bigChunk;

void setClock() {
  NTP.begin("pool.ntp.org", "time.nist.gov");
  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}

int incomingByte = 0; // for incoming serial data
int sendCommand(const char* command) {
  Serial.print("Sending: ");
  Serial.println(command);
  Serial1.println(command);
  // wait for reply 
  while(Serial.available() == 0) {
    incomingByte = Serial1.read();
    Serial.print("I received: ");
    Serial.println(incomingByte, DEC);
    return incomingByte;
    //delay(500);
  }
  return 0;  
}

void updateStatus() {
  int q = sendCommand("q");
  LAMPpixels.clear();
  if (q == 49) {
    open = true;
  for (int i = 0; i < LNUMPIXELS; i++) {
    LAMPpixels.setPixelColor(i, LAMPpixels.Color(0, 255, 0));
    
  }     
  } else {
    open = false;
   for (int i = 0; i < LNUMPIXELS; i++) {
    LAMPpixels.setPixelColor(i, LAMPpixels.Color(255, 0, 0));
    
  }  
  }
  LAMPpixels.show();
  //delay(5000);
  time_t now;
  uhrzeit = time(&now);
  uhrzeit += 5400;
  Serial.println(uhrzeit);
  Serial.println(open);
}

void handleRoot() {
  server.send(200, "text/plain", "hello from esp8266!\r\n");
}

void openclosenow() {
  String json = "{ \"state\": " + String(open ? "true" : "false") + " }";
  server.send(200, "application/json", json);
}

void openclosechange() {
  String json = "{ \"state\": " + String(open ? "true" : "false") + ", \"next_change\": \"" + String(uhrzeit) + "\" }";
  server.send(200, "application/json", json);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: " + server.uri() + "\nMethod: " + (server.method() == HTTP_GET ? "GET" : "POST") + "\nArguments: " + String(server.args()) + "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void handleChunked() {
  server.chunkedResponseModeStart(200, F("text/html"));
  server.sendContent(bigChunk);
  server.sendContent(F("chunk 2"));
  server.sendContent(bigChunk);
  server.chunkedResponseFinalize();
}

void setupOTA() {
  ArduinoOTA.setHostname("TerrorHebel");
  //ArduinoOTA.setPassword("clubmate");

  ArduinoOTA.onStart([]() {
    String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
}

void setupServer() {
  server.on("/", handleRoot);
  server.on("/open-close/now", openclosenow);
  server.on("/open-close/now/", openclosenow);
  server.on("/open-close/next-change", openclosechange);
  server.on("/open-close/next-change/", openclosechange);
  server.onNotFound(handleNotFound);
  server.begin();
}

void setup() {
  Serial.begin(9600);
  Serial.println("Booting");
  Serial1.begin(9600);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    rp2040.restart();
  }

  setupOTA();
  setupServer();

  LAMPpixels.begin();
  LAMPpixels.clear();

  setClock();
    for (int i = 0; i < LNUMPIXELS; i++) {
    LAMPpixels.setPixelColor(i, LAMPpixels.Color(255, 255, 255));
    LAMPpixels.show();
  }
}




void loop() {
  ArduinoOTA.handle();
  server.handleClient();


  //sendCommand("q");  
  updateStatus();

  //delay(500);
}