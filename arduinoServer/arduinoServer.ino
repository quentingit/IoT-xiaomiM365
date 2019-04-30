#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <WiFiClientSecure.h>
#include <Adafruit_NeoPixel.h> // LED


////////////////////////////////////////////////////////////////////////////////
const char* ssid = "ESGI";//"CCCP";                     // TODO: your network SSID (name)
const char* password = "Reseau-GES";//"6c28995d5791";             // TODO: your network password
//const IPAddress outIp(/*192,168,43,222*/);    // TODO: remote IP of your computer
////////////////////////////////////////////////////////////////////////////////

#define PIN D8
#define SIZE 14
Adafruit_NeoPixel strip = Adafruit_NeoPixel(14, PIN, NEO_GRB + NEO_KHZ800); // 14 LEDS in the strip
int buttonPin[SIZE] = {D0, D1, D2, D3, D4, D5, D6, D7};
bool blink = false;

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint16_t start, uint16_t end) {
  for (uint16_t i = start; i < end; i++) {
    strip.setPixelColor(i, c);
  }

  strip.show();
}
void turnOff() {
  colorWipe(strip.Color(0, 0, 0),0,14);
}
void turnRight() {
  colorWipe(strip.Color(255, 100, 0), 0, 7); // 7 leds at right ->
}
void turnLeft() {
  colorWipe(strip.Color(255, 100, 0), 7, 14); // 7 leds at left <-
}

/*const unsigned int localPort = 8888;  // local port to listen for OSC packets (not used for sending)
const unsigned int outPort = 4559;    // remote port to receive OSC
WiFiUDP Udp;                          // A UDP instance to let us send and receive packets over UDP
*/

const uint16_t HTTPPort = 80;
const byte maxURL = 50;
char urlRequest[maxURL + 1]; // +1 for '\0'
long timeThresold = 0;
WiFiClientSecure clientSecure;
WiFiServer serveurWeb(HTTPPort); // create http server on standard port

void setup() {
    Serial.begin(115200);
    // Light the leds
    for (int i = 0; i < 14; i++) {
      pinMode(buttonPin[i], INPUT_PULLUP);
      Serial.println(i);
    }
    strip.begin();
    strip.show(); // Initialize all pixels to 'off'
    
    // Connect to WiFi network
    // WiFi.begin(ssid, pass);

    /*while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }*/

    // Udp.begin(localPort);

    
}

void loop() {
    static float cnt = 0.42;
    OSCMessage msg("/test");
    msg.add(cnt++);
    //Udp.beginPacket(outIp, outPort);
    //msg.send(Udp);
    //Udp.endPacket();
    //msg.empty();
    if(blink==true){ turnRight(); blink=false;  Serial.println("Allume"); }
    else{ turnOff(); blink=true; Serial.println("Eteint"); }

    delay(400);
}
