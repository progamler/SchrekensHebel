#include <ESP8266WebServer.h>  // Bibliothek für Webserver-Funktionalität
#include <WiFi.h>              // Bibliothek für WLAN-Funktionalität
#include <ArduinoOTA.h>        // Bibliothek für Over-the-Air Updates
#include <time.h>              // Bibliothek für Zeitfunktionen
#include <Adafruit_NeoPixel.h> // Bibliothek für NeoPixel-LEDs

#define LAMPPIN 5    // Pin für Lampen-LEDs
#define CASEPIN 4    // Pin für Case-LEDs
#define LNUMPIXELS 7 // Anzahl der Lampen-LEDs
#define CNUMPIXELS 4 // Anzahl der Case-LEDs


#ifndef STASSID
#define STASSID "XXX"              // WLAN-SSID
#define STAPSK "XXX"          // WLAN-Passwort
#define STAMAC "d8:3a:dd:21:ca:0d" // WLAN-MAC-Adresse
#define WIFIIP IPAddress(10, 42, 131, 216)  // WLAN-IP-Adresse
#define WIFIMASK IPAddress(255, 255, 255, 0)  // WLAN-Subnetzmaske
#define WIFIGW IPAddress(10, 42, 131, 1)    // WLAN-Gateway
#define WIFIDNS IPAddress(8, 8, 8, 8)      // WLAN-DNS-Server
#endif

const char *ssid = STASSID;    // SSID als Konstante
const char *password = STAPSK; // Passwort als Konstante

bool open = false;                                                        // Status: offen oder geschlossen
long int uhrzeit;                                                         // Zeitvariable für nächste Änderung
ESP8266WebServer server(80);                                              // Webserver auf Port 80
Adafruit_NeoPixel LAMPpixels(LNUMPIXELS, LAMPPIN, NEO_GRBW + NEO_KHZ800); // Lampen-LEDs
Adafruit_NeoPixel CASEpixels(CNUMPIXELS, CASEPIN, NEO_GRBW + NEO_KHZ800); // Case-LEDs

String bigChunk; // Beispiel-String für Chunked Response

// Funktion zum Setzen der Systemzeit per NTP
void setClock()
{
  NTP.begin("pool.ntp.org", "time.nist.gov"); // Starte NTP mit Servern
  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr); // Hole aktuelle Zeit
  while (now < 8 * 3600 * 2)
  { // Warte bis Zeit synchronisiert ist
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo); // Hole Zeitstruktur
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo)); // Ausgabe der aktuellen Zeit
}

// Funktion zum Senden eines Kommandos über die serielle Schnittstelle
int sendCommand(const char *command)
{
  Serial.print("Sending: ");
  Serial.println(command);
  Serial1.println(command); // Sende Kommando an Serial1
  while (Serial.available() == 0)
  { // Warte auf Antwort
    int incomingByte = Serial1.read();
    Serial.print("I received: ");
    Serial.println(incomingByte, DEC);
    return incomingByte; // Rückgabe des empfangenen Bytes
  }
  return 0;
}

// Setzt den Status der Lampen-LEDs je nach Offen/Zu
void setLampStatus(bool isOpen)
{
  LAMPpixels.clear();                                                                  // Alle LEDs ausschalten
  uint32_t color = isOpen ? LAMPpixels.Color(0, 255, 0) : LAMPpixels.Color(255, 0, 0); // Grün wenn offen, rot wenn zu
  for (int i = 0; i < LNUMPIXELS; i++)
  {
    LAMPpixels.setPixelColor(i, color); // Setze Farbe für jede LED
  }
  LAMPpixels.show(); // Zeige die Änderung an
}

// Aktualisiert den Status (offen/zu) und setzt LEDs entsprechend
void updateStatus()
{
  static int lastQ = -1;    // Letzter Status
  int q = sendCommand("q"); // Frage aktuellen Status ab
  // Prüfe auf Änderung und ob q 0, 1 oder '?' ist
  if (q != lastQ && (q == 0 || q == 1 || q == '?'))
  {
    switch (q)
    {
    case 0:
      Serial.println("Status: closed"); // Status "zu"
      // sendCommand("<CHAR>"); // TODO: Ersetze <CHAR> durch das gewünschte Zeichen
      break;
    case 1:
      Serial.println("Status: open"); // Status "offen"
      // sendCommand("<CHAR>"); // TODO: Ersetze <CHAR> durch das gewünschte Zeichen
      break;
    case '?':
      Serial.println("Status: unknown"); // Status unbekannt
      // sendCommand("<CHAR>"); // TODO: Ersetze <CHAR> durch das gewünschte Zeichen
      break;
    }
  }
  lastQ = q;           // Speichere aktuellen Status
  open = (q == 49);    // Setze open auf true wenn q == 49 ('1')
  setLampStatus(open); // Aktualisiere LEDs
  time_t now;
  uhrzeit = time(&now) + 5400; // Setze nächste Änderungszeit (Beispielwert)
  Serial.println(uhrzeit);     // Debug-Ausgabe
  Serial.println(open);        // Debug-Ausgabe
}

// Sendet den aktuellen Status als JSON an den Client
void sendJsonState(bool withChange = false)
{
  String json = "{ \"state\": " + String(open ? "true" : "false");
  if (withChange)
  {
    json += ", \"next_change\": \"" + String(uhrzeit) + "\"";
  }
  json += " }";
  server.send(200, "application/json", json); // Sende JSON-Antwort
}

// Handler für die Root-URL
void handleRoot()
{
  server.send(200, "text/plain", "hello from esp8266!\r\n"); // Sende einfachen Text
}

// Handler für /open-close/now
void openclosenow()
{
  sendJsonState(false); // Sende Status ohne nächste Änderung
}

// Handler für /open-close/next-change
void openclosechange()
{
  sendJsonState(true); // Sende Status mit nächster Änderung
}

// Handler für /spaceapi
void handleSpaceAPI()
{
  String json = "{\n";
  json += "  \"api\": \"0.15\",\n";
  json += "  \"space\": \"C-Hack\",\n";
  json += "  \"logo\": \"\",\n";
  json += "  \"url\": \"https://c-hack.de/\",\n";
  json += "  \"location\": {\n";
  json += "    \"address\": \"Im Zwinger 4, 75365 Calw, Deutschland\",\n";
  json += "    \"lat\": 48.7131,\n";
  json += "    \"lon\": 8.7366\n";
  json += "  },\n";
  json += "  \"contact\": {\n";
  json += "    \"email\": \"info@c-hack.de\",\n";
  // json += "    \"twitter\": \"@chackcw\",\n";
  // json += "    \"ml\": \"https://lists.c-hack.de/postorius/lists/c-hack.c-hack.de/\",\n";
  // json += "    \"irc\": \"irc://irc.hackint.org/#c-hack\",\n";
  // json += "    \"matrix\": \"#c-hack:matrix.org\"\n";
  json += "  },\n";
  json += "  \"state\": {\n";
  json += "    \"open\": " + String(open ? "true" : "false") + "\n";
  json += "  },\n";
  json += "  \"projects\": [\n";
  json += "    \"https://c-turm.c-hack.de\",\n";
  json += "    // Weitere Projekt-URLs können hier ergänzt werden\n";
  json += "  ]\n";
  json += "  // Weitere Felder können hier einfach ergänzt werden\n";
  json += "}\n";
  server.send(200, "application/json", json);
}

// Handler für nicht gefundene Seiten
void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: " + server.uri() + "\nMethod: " + (server.method() == HTTP_GET ? "GET" : "POST") + "\nArguments: " + String(server.args()) + "\n";
  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message); // Sende Fehlernachricht
}

// Beispiel für Chunked Response
void handleChunked()
{
  server.chunkedResponseModeStart(200, F("text/html")); // Starte Chunked Response
  server.sendContent(bigChunk);                         // Sende ersten Chunk
  server.sendContent(F("chunk 2"));                     // Sende zweiten Chunk
  server.sendContent(bigChunk);                         // Sende dritten Chunk
  server.chunkedResponseFinalize();                     // Beende Chunked Response
}

// Initialisiert OTA (Over-the-Air Update)
void setupOTA()
{
  ArduinoOTA.setHostname("TerrorHebel"); // Setze Hostname

  ArduinoOTA.onStart([]()
                     {
                       String type = (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem";
                       Serial.println("Start updating " + type); // Ausgabe Update-Typ
                     });
  ArduinoOTA.onEnd([]()
                   {
                     Serial.println("\nEnd"); // Ausgabe bei Ende
                   });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                        {
                          Serial.printf("Progress: %u%%\r", (progress / (total / 100))); // Fortschrittsanzeige
                        });
  ArduinoOTA.onError([](ota_error_t error)
                     {
    Serial.printf("Error[%u]: ", error); // Fehlerausgabe
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
    } });
  ArduinoOTA.begin(); // Starte OTA
}

// Initialisiert den Webserver und registriert die Handler
void setupServer()
{
  server.on("/", handleRoot);                             // Root-Handler
  server.on("/open-close/now", openclosenow);             // Handler für aktuellen Status
  server.on("/open-close/now/", openclosenow);            // Handler für aktuellen Status (mit Slash)
  server.on("/open-close/next-change", openclosechange);  // Handler für nächste Änderung
  server.on("/open-close/next-change/", openclosechange); // Handler für nächste Änderung (mit Slash)
  server.on("/spaceapi", handleSpaceAPI);                 // SpaceAPI-Handler
  server.onNotFound(handleNotFound);                      // Handler für nicht gefundene Seiten
  server.begin();                                         // Starte Server
}

// Setup-Funktion, wird einmal beim Start ausgeführt
void setup()
{
  Serial.begin(115200);      // Starte serielle Kommunikation
  Serial.println("Booting"); // Debug-Ausgabe
  Serial1.begin(9600);       // Starte zweite serielle Schnittstelle

  // Setze die MAC-Adresse vor dem Verbindungsaufbau
  uint8_t mac[6];
  sscanf(STAMAC, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]); // Lese MAC aus String
  WiFi.macAddress(mac);                                                                                  // optional: aktuelle MAC auslesen
  //WiFi.setMacAddress(mac);                                                                               // Setze MAC-Adresse

  // Verbinde mit dem WLAN
  WiFi.mode(WIFI_STA);        // Setze WLAN-Modus auf Station
  WiFi.begin(ssid, password); // Verbinde mit WLAN
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  { // Warte auf Verbindung
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    rp2040.restart(); // Neustart bei Fehler
  }
  wifi.config(WIFIIP, WIFIMASK, WIFIGW, WIFIDNS); // Setze IP-Adresse, Subnetzmaske, Gateway und DNS-Server
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP()); // Ausgabe der IP-Adresse
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress()); // Ausgabe der MAC-Adresse
  Serial.print("DNS address: ");
  Serial.println(WiFi.dnsIP()); // Ausgabe der DNS-Adresse
  Serial.print("Gateway address: ");
  Serial.println(WiFi.gatewayIP()); // Ausgabe der Gateway-Adresse
  Serial.print("Subnet mask: ");
  Serial.println(WiFi.subnetMask()); // Ausgabe der Subnetzmaske
  Serial.print("WiFi signal strength: ");
  Serial.println(WiFi.RSSI()); // Ausgabe der WLAN-Signalstärke
  Serial.print("WiFi channel: ");
  Serial.println(WiFi.channel()); // Ausgabe des WLAN-Kanals
  Serial.print("WiFi mode: ");
  Serial.println(WiFi.getMode()); // Ausgabe des WLAN-Modus
  Serial.print("WiFi hostname: ");
  Serial.println(WiFi.getHostname()); // Ausgabe des Hostnamens
  

  setupOTA();    // Initialisiere OTA
  setupServer(); // Initialisiere Webserver

  LAMPpixels.begin(); // Initialisiere Lampen-LEDs
  LAMPpixels.clear(); // Schalte alle LEDs aus

  setClock();           // Setze Systemzeit per NTP
  setLampStatus(false); // Setze Lampenstatus auf "zu"
}

// Hauptschleife, wird ständig ausgeführt
void loop()
{
  ArduinoOTA.handle();   // Bearbeite OTA-Events
  server.handleClient(); // Bearbeite Webserver-Requests
  updateStatus();        // Aktualisiere Status und LEDs
}