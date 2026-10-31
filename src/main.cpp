#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN      13
#define NUM_LEDS     8

// WiFi AP Settings
const char *ssid = "CJMCU-POV";
const char *password = "12345678";

// Create Web Server on port 80
WebServer server(80);
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Global variables for on-the-fly adjustment
String currentText = "POV";
int sliceDelayMicros = 800; 
int wordGapMillis    = 10;  // Traditional fixed delay after text frame
int currentBrightness = 220;

// Default color RGB (Green: R=0, G=255, B=0)
byte colorR = 0;
byte colorG = 255;
byte colorB = 0;
String currentHexColor = "#00ff00";

// Corrected 8-bit 2D Font Array (A-Z, 0-9, Space, Å, Ä, Ö)
const byte font8[][5] = {
  {0x3F, 0x48, 0x48, 0x48, 0x3F}, // 0:  A
  {0x7F, 0x49, 0x49, 0x49, 0x36}, // 1:  B
  {0x3E, 0x41, 0x41, 0x41, 0x22}, // 2:  C
  {0x7F, 0x41, 0x41, 0x22, 0x1C}, // 3:  D
  {0x7F, 0x49, 0x49, 0x49, 0x41}, // 4:  E
  {0x7F, 0x09, 0x09, 0x09, 0x01}, // 5:  F
  {0x3E, 0x41, 0x49, 0x49, 0x7A}, // 6:  G
  {0x7F, 0x08, 0x08, 0x08, 0x7F}, // 7:  H
  {0x00, 0x41, 0x7F, 0x41, 0x00}, // 8:  I
  {0x20, 0x40, 0x41, 0x3F, 0x01}, // 9:  J
  {0x7F, 0x08, 0x14, 0x22, 0x41}, // 10: K
  {0x7F, 0x40, 0x40, 0x40, 0x40}, // 11: L
  {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // 12: M
  {0x7F, 0x04, 0x08, 0x10, 0x7F}, // 13: N
  {0x3E, 0x41, 0x41, 0x41, 0x3E}, // 14: O
  {0x7F, 0x09, 0x09, 0x09, 0x06}, // 15: P
  {0x3E, 0x41, 0x51, 0x21, 0x5E}, // 16: Q
  {0x7F, 0x09, 0x19, 0x29, 0x46}, // 17: R
  {0x46, 0x49, 0x49, 0x49, 0x31}, // 18: S
  {0x01, 0x01, 0x7F, 0x01, 0x01}, // 19: T
  {0x3F, 0x40, 0x40, 0x40, 0x3F}, // 20: U
  {0x1F, 0x20, 0x40, 0x20, 0x1F}, // 21: V
  {0x3F, 0x40, 0x38, 0x40, 0x3F}, // 22: W
  {0x63, 0x14, 0x08, 0x14, 0x63}, // 23: X
  {0x07, 0x08, 0x70, 0x08, 0x07}, // 24: Y
  {0x61, 0x51, 0x49, 0x45, 0x43}, // 25: Z
  {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 26: 0
  {0x00, 0x42, 0x7F, 0x40, 0x00}, // 27: 1
  {0x42, 0x61, 0x51, 0x49, 0x46}, // 28: 2
  {0x21, 0x41, 0x45, 0x4B, 0x31}, // 29: 3
  {0x18, 0x14, 0x12, 0x7F, 0x10}, // 30: 4
  {0x27, 0x45, 0x45, 0x45, 0x39}, // 31: 5
  {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 32: 6
  {0x01, 0x71, 0x09, 0x05, 0x03}, // 33: 7
  {0x36, 0x49, 0x49, 0x49, 0x36}, // 34: 8
  {0x06, 0x49, 0x49, 0x29, 0x1E}, // 35: 9
  {0x00, 0x00, 0x00, 0x00, 0x00}, // 36: [Space]
  {0x3E, 0x49, 0x4A, 0x49, 0x3E}, // 37: Å
  {0x3D, 0x48, 0x48, 0x48, 0x3D}, // 38: Ä
  {0x3C, 0x41, 0x41, 0x41, 0x3C}  // 39: Ö
};

// Function to render a single vertical column on the LED stick
void drawColumn(byte columnPattern) {
  for (int led = 0; led < NUM_LEDS; led++) {
    if ((columnPattern >> led) & 0x01) {
      strip.setPixelColor(led, strip.Color(colorR, colorG, colorB));
    } else {
      strip.setPixelColor(led, strip.Color(0, 0, 0));
    }
  }
  strip.show();
  delayMicroseconds(sliceDelayMicros);
}

// Generate English Web UI Layout
void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<style>body{font-family:Arial;text-align:center;background:#222;color:#fff;padding:20px;}";
  html += ".card{background:#333;padding:20px;border-radius:10px;display:inline-block;max-width:90%;width:350px;box-shadow: 0 4px 8px rgba(0,0,0,0.3);}";
  html += "input[type='text'], input[type='number'], input[type='color'], input[type='range']{padding:10px;font-size:16px;width:90%;border-radius:5px;border:1px solid #555;background:#444;color:#fff;margin-bottom:15px;box-sizing:border-box;}";
  html += "input[type='color']{height:50px;cursor:pointer;}";
  html += "label{display:block;text-align:left;margin-left:5%;font-weight:bold;font-size:14px;color:#bbb;margin-bottom:5px;}";
  html += "input[type='submit']{padding:12px;font-size:18px;background:#00ff00;color:#000;border:none;border-radius:5px;cursor:pointer;width:90%;font-weight:bold;}";
  html += "hr{border:0;height:1px;background:#555;margin:20px 0;}";
  html += "</style></head><body>";
  html += "<div class='card'>";
  html += "<h2>CJMCU POV Settings</h2>";
  html += "<p>Active Text: <span style='color:" + currentHexColor + ";font-weight:bold;'>" + currentText + "</span></p>";
  
  html += "<form action='/update' method='POST'>";
  
  html += "<label>Text (A-Z, 0-9, Å, Ä, Ö):</label>";
  html += "<input type='text' name='text' value='" + currentText + "' maxlength='15'>";
  
  html += "<label>Text Color (RGB):</label>";
  html += "<input type='color' name='color' value='" + currentHexColor + "'>";
  
  html += "<label>Brightness (10 - 255):</label>";
  html += "<input type='range' name='brightness' min='10' max='255' value='" + String(currentBrightness) + "'>";
  
  html += "<hr>";
  
  html += "<label>Slice Delay (&micro;s):</label>";
  html += "<input type='number' name='slice' value='" + String(sliceDelayMicros) + "' min='200' max='5000'>";
  
  html += "<label>Word Gap Delay (ms):</label>";
  html += "<input type='number' name='gap' value='" + String(wordGapMillis) + "' min='1' max='200'>";
  
  html += "<input type='submit' value='UPDATE INSTANTLY'>";
  html += "</form></div></body></html>";
  
  server.send(200, "text/html; charset=UTF-8", html);
}

// Process forms inputs and update global variables instantly
void handleUpdate() {
  if (server.hasArg("text")) {
    currentText = server.arg("text");
    currentText.trim();
    if (currentText.length() == 0) currentText = "POV";
  }
  
  // Convert HTML Color Hex (#RRGGBB) into separate RGB bytes
  if (server.hasArg("color")) {
    currentHexColor = server.arg("color");
    String hex = currentHexColor;
    hex.replace("#", "0x");
    long number = strtol(hex.c_str(), NULL, 16);
    colorR = (number >> 16) & 0xFF;
    colorG = (number >> 8) & 0xFF;
    colorB = number & 0xFF;
  }
  
  if (server.hasArg("brightness")) {
    currentBrightness = server.arg("brightness").toInt();
    strip.setBrightness(currentBrightness);
  }
  
  if (server.hasArg("slice")) {
    sliceDelayMicros = server.arg("slice").toInt();
    if (sliceDelayMicros < 200) sliceDelayMicros = 200;
  }
  
  if (server.hasArg("gap")) {
    wordGapMillis = server.arg("gap").toInt();
    if (wordGapMillis < 1) wordGapMillis = 1;
  }

  // Redirect client back to index page after processing parameters
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  strip.begin();
  strip.show();
  strip.setBrightness(currentBrightness);

  // Initialize enclosed WiFi Access Point
  WiFi.softAP(ssid, password);
  
  // Register routing entry paths for Web Server
  server.on("/", HTTP_GET, handleRoot);
  server.on("/update", HTTP_POST, handleUpdate);
  server.onNotFound([]() { server.send(404, "text/plain", "Not Found"); });
  
  server.begin();
}

void loop() {
  // Listen and handle incoming web server client requests
  server.handleClient();

  // Parse text character-by-character supporting 2-byte UTF-8 scandinavian inputs
  unsigned int i = 0;
  while (i < currentText.length()) {
    int fontIndex = 36; // Default to space
    byte c = currentText[i];

    if (c == 0xC3) { // UTF-8 scandinavian lead byte detected
      if (i + 1 < currentText.length()) {
        byte nextC = currentText[i + 1];
        if (nextC == 0x85 || nextC == 0xA5) fontIndex = 37;      // Å / å
        else if (nextC == 0x84 || nextC == 0xA4) fontIndex = 38; // Ä / ä
        else if (nextC == 0x96 || nextC == 0xB6) fontIndex = 39; // Ö / ö
        i += 2; // Jump over both evaluated UTF-8 bytes
      } else {
        i++;
      }
    } else { // Standard ASCII tracking characters (A-Z, 0-9)
      if (c >= 'A' && c <= 'Z') fontIndex = c - 'A';
      else if (c >= 'a' && c <= 'z') fontIndex = c - 'a';
      else if (c >= '0' && c <= '9') fontIndex = c - '0' + 26;
      i++;
    }
    
    // Draw 5 vertical patterns for selected character index
    for (int col = 0; col < 5; col++) {
      drawColumn(font8[fontIndex][col]);
    }
    drawColumn(0x00); // Small visual pixel buffer gap between letters
  }
  
  // Turn off all LED elements at the end of text array frame loop
  strip.clear();
  strip.show();
  
  // Frame loop delay before next refresh cycle
  delay(wordGapMillis);
}
