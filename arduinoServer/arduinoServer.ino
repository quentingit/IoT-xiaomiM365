//////////////////////////////////////////////////////////////////////////////// LEDS
#include <Adafruit_NeoPixel.h>
//////////////////////////////////////////////////////////////////////////////// BLIND SPOT SENSOR
// TIMEOUT
const unsigned long MEASURE_TIMEOUT = 12000UL; // 12ms = ~4m (340m/s)

/* Vitesse du son dans l'air en mm/us */
const float SOUND_SPEED = 340.0 / 1000;
//////////////////////////////////////////////////////////////////////////////// WIFI
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <WiFiClientSecure.h>
const char* ssid = "ESGI";//"CCCP";                     // TODO: your network SSID (name)
const char* password = "Reseau-GES";//"6c28995d5791";             // TODO: your network password
//const IPAddress outIp(/*192,168,43,222*/);    // TODO: remote IP of your computer

//////////////////////////////////////////////////////////////////////////////// LEDS
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
  colorWipe(strip.Color(255, 70, 0), 0, 7); // 7 leds orange at right ->
}
void turnLeft() {
  colorWipe(strip.Color(255, 70, 0), 7, 14); // 7 leds orange at left <-
}
void Break() {
  colorWipe(strip.Color(255, 0, 0), 0, 14);
}
int delaycount = 0;
int action = 0;
/////////////////////////////////////////////////////////////////////////////// BLIND SPOT SENSOR
int delaylight = 0;
/////////////////////////////////////////////////////////////////////////////// WIFI
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
    ////////////////////////////////////////////////////////// TURNING INDICATORS INIT
    // Light the leds
    for (int i = 0; i < 14; i++) {
      pinMode(buttonPin[i], INPUT_PULLUP);
      Serial.println(i);
    }
    strip.begin();
    strip.show(); // Initialize all leds to 'off'
    ////////////////////////////////////////////////////////// BLIND SPOT INIT
    pinMode(D4, OUTPUT); // TRIGGER_PIN
    digitalWrite(D4, LOW); // TRIGGER_PIN doit être à LOW au repos
    pinMode(D3, INPUT); // ECHO_PIN
    
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
    // ===================================== BLIND SPOT DETECTION
    /* start to measure with HIGH PULSE of 10µs in TRIGGER PIN */
    digitalWrite(D4, HIGH); // D4 = trigger pin
    delayMicroseconds(10);
    digitalWrite(D4, LOW); // D4 = trigger pin
    
    /* measure time between pulse and ECHO */
    long measure = pulseIn(D3, HIGH, MEASURE_TIMEOUT); // ECHO_PIN
     
    /* calculation of the distance... */
    float distance_cm = (measure / 2.0 * SOUND_SPEED) / 10.0;

    Serial.println("400"); // MAX
    if(distance_cm) {
      Serial.println(distance_cm, 2); // CM DISTANCE
      if( distance_cm < 120 ) { 
        Break(); // RED LIGHTS ARE ON
        delaylight = 0; // RESET LIGHT DELAY
      }
    }
    if( !action && delaylight > 10 || !action && distance_cm >= 120 && delaylight > 5) {
      turnOff();
    }
    // ===================================== TURN LIGHT INDICATION
    if(delaycount == 10) delaycount = 0;
    if(delaycount == 0){
      if(blink==true){ 
        if( action==1 ) turnLeft();
        if( action==2 ) turnRight();
        blink=false;  /*Serial.println("Allume"); }*/
      }
      else{ 
        if( distance_cm >= 120 ) { 
          turnOff(); 
        } 
        blink=true; /*Serial.println("Eteint");*/ }
    }

    // =====================================
    delaycount++;
    delaylight++;
    delay(40);
}
