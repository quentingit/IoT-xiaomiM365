// ===================================== LEDS
#include <Adafruit_NeoPixel.h>
// ===================================== BLIND SPOT SENSOR
#include "QuickMedianLib.h"
float valuesHistory[15];
float valuesHistoryLen = sizeof(valuesHistory) / sizeof(float);
// ===================================== WIFI
#include <ESP8266WiFi.h>
// ===================================== LEDS
#define PIN D8
#define SIZE 14
Adafruit_NeoPixel strip = Adafruit_NeoPixel(14, PIN, NEO_GRB + NEO_KHZ800); // 14 LEDS in the strip
bool blink = false;

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint16_t start, uint16_t end) {
  for (uint16_t i = start; i < end; i++) {
    strip.setPixelColor(i, c);
  }

  strip.show();
}

void turnOff() {
  colorWipe(strip.Color(0, 0, 0), 0, 14);
}

void turnRight() {
  colorWipe(strip.Color(255, 70, 0), 0, 7); // 7 leds orange at right ->
}

void turnLeft() {
  colorWipe(strip.Color(255, 70, 0), 7, 14); // 7 leds orange at left <-
}

void Break() {
  colorWipe(strip.Color(255, 0, 0), 0, 14); // 14 red warning leds
}

int delaycount = 0;
int action = 0;
// ===================================== BLIND SPOT SENSOR
const unsigned long MEASURE_TIMEOUT = 25000UL; // TIMEOUT : 12ms = ~4m (340m/s)
const float SOUND_SPEED = 340.0 / 1000; // sound speed in the air in mm/µs
int delaylight = 0;
// ===================================== WIFI
const char* ssid = "ESGI";//"CCCP";                     // your network SSID (name)
const char* password = "Reseau-GES";//"6c28995d5791";             // your network password
WiFiServer server(80);
// Variable to store the HTTP request
String header;
// Auxiliar variables to store the current output state
String output5State = "off";
String output4State = "off";

// Assign output variables to GPIO pins
const int output5 = 5;
const int output4 = 4;
// ===================================== WEB SERVER
const uint16_t HTTPPort = 80;
const byte maxURL = 50;
char urlRequest[maxURL + 1]; // +1 for '\0'
long timeThresold = 0;
WiFiClientSecure clientSecure;
WiFiServer serveurWeb(HTTPPort); // create http server on standard port

void printHTTPServerInfo()
{
  Serial.println("==============================");
  Serial.print(F("SERVER URL: http://"));
  Serial.print(WiFi.localIP());
  if (HTTPPort != 80) {
    Serial.print(F(":"));
    Serial.print(HTTPPort);
  }
  Serial.println();
  Serial.println("==============================");
}

void requestListener()
{
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // turns the GPIOs on and off
            if (header.indexOf("GET /5/on") >= 0) {
              Serial.println("GPIO 5 on");
              output5State = "on";
              digitalWrite(output5, HIGH);
            } else if (header.indexOf("GET /5/off") >= 0) {
              Serial.println("GPIO 5 off");
              output5State = "off";
              digitalWrite(output5, LOW);
            } else if (header.indexOf("GET /4/on") >= 0) {
              Serial.println("GPIO 4 on");
              output4State = "on";
              digitalWrite(output4, HIGH);
            } else if (header.indexOf("GET /4/off") >= 0) {
              Serial.println("GPIO 4 off");
              output4State = "off";
              digitalWrite(output4, LOW);
            }

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");

            // Web Page Heading
            client.println("<body><h1>ESP8266 Web Server</h1>");

            // Display current state, and ON/OFF buttons for GPIO 5
            client.println("<p>GPIO 5 - State " + output5State + "</p>");
            // If the output5State is off, it displays the ON button
            if (output5State == "off") {
              client.println("<p><a href=\"/5/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/5/off\"><button class=\"button button2\">OFF</button></a></p>");
            }

            // Display current state, and ON/OFF buttons for GPIO 4
            client.println("<p>GPIO 4 - State " + output4State + "</p>");
            // If the output4State is off, it displays the ON button
            if (output4State == "off") {
              client.println("<p><a href=\"/4/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/4/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  } else {
    Serial.println("No client.");
  }
}


// ==================================== SETUP

void setup() {
  Serial.begin(115200);
  // ===================================== TURNING INDICATORS INIT
  strip.begin();
  strip.show(); // Initialize all leds to 'off'
  // ===================================== BLIND SPOT INIT
  pinMode(D2, OUTPUT); // TRIGGER_PIN
  digitalWrite(D2, LOW); // TRIGGER_PIN should be LOW when not using
  pinMode(D1, INPUT); // ECHO_PIN

  // Connect to WiFi network
  WiFi.begin(ssid, password);

  Serial.print("CONNECTING TO WIFI");
  while (WiFi.status() != WL_CONNECTED) { // while not connected, we wait
    delay(500);
    Serial.print('.');
  }
  Serial.println();

  // start web server
  serveurWeb.begin();
  printHTTPServerInfo();

  delay(5000); // so we can see the server informations
}

void loop() {
  // ===================================== HTTP SERVER
  requestListener();
  // ===================================== BLIND SPOT DETECTION
  /* start to measure with HIGH PULSE of 10µs in TRIGGER PIN */
  digitalWrite(D2, HIGH); // trigger pin
  delayMicroseconds(10);
  digitalWrite(D2, LOW); // trigger pin

  /* measure time between pulse and ECHO */
  long measure = pulseIn(D1, HIGH, MEASURE_TIMEOUT); // ECHO_PIN

  /* calculation of the distance... */
  float distance_cm = (measure / 2.0 * SOUND_SPEED) / 10.0;

  //Serial.println("0 400 "); // MAX
  Serial.println();
  if (distance_cm)
  {
    // serial print median + average
    for(int i = 0;i<15;i++)
    {
      if(!valuesHistory[13-i]) valuesHistory[14-i] = distance_cm;
      valuesHistory[14-i] = valuesHistory[14-i-1];
    }
    valuesHistory[0] = distance_cm;
    float med = QuickMedian<float>::GetMedian(valuesHistory, valuesHistoryLen);
    Serial.print(med); // CM DISTANCE MEDIAN + AVERAGE
    Serial.print(" ");
    Serial.println(distance_cm, 2); // CM DISTANCE
    if ( med < 120 )
    {
      Break(); // RED LIGHTS ARE ON
      delaylight = 0; // RESET LIGHT DELAY
    }
  }
  if ( !action && delaylight > 10 || !action && distance_cm >= 120 && delaylight > 5)
  {
    turnOff();
  }
  // ===================================== TURN LIGHT INDICATION
  if (delaycount == 10) delaycount = 0;
  if (delaycount == 0)
  {
    if (blink == true)
    {
      if ( action == 1 ) turnLeft();
      if ( action == 2 ) turnRight();
      blink = false;
    }
    else
    {
      if ( distance_cm >= 120 ) {
        turnOff();
      }
      blink = true;
    }
  }

  // ===================================== COUNTERS
  delaycount++;
  delaylight++;

  delay(40);
}
