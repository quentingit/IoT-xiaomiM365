// ===================================== LEDS
#include <Adafruit_NeoPixel.h>
// ===================================== BLIND SPOT SENSOR
#include "QuickMedianLib.h"
float valuesHistory[20];
float valuesHistoryLen = sizeof(valuesHistory) / sizeof(float); // see doc for QuickMedianLib
int HistorySize = 20;
// ===================================== WIFI + SERVER + FS
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>

#define DBG_OUTPUT_PORT Serial

#ifndef STASSID
#define STASSID "a"
#define STAPSK  "lalalalala"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;
const char* host = "esp8266fs";

ESP8266WebServer server(80);
//holds the current upload
File fsUploadFile;

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

void warnings() {
  colorWipe(strip.Color(255, 70, 0), 0, 14); // 14 orange warning leds <->
}

void Break() {
  colorWipe(strip.Color(255, 0, 0), 0, 14); // 14 red warning leds <->
}

int delaycount = 0;
int action = 0;
// ===================================== BLIND SPOT SENSOR
const unsigned long MEASURE_TIMEOUT = 25000UL; // TIMEOUT : 12ms = ~4m (340m/s)
const float SOUND_SPEED = 340.0 / 1000; // sound speed in the air in mm/µs
int delaylight = 0;
// =====================================
//format bytes
String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}

String getContentType(String filename) {
  if (server.hasArg("download")) {
    return "application/octet-stream";
  } else if (filename.endsWith(".htm")) {
    return "text/html";
  } else if (filename.endsWith(".html")) {
    return "text/html";
  } else if (filename.endsWith(".css")) {
    return "text/css";
  } else if (filename.endsWith(".js")) {
    return "application/javascript";
  } else if (filename.endsWith(".png")) {
    return "image/png";
  } else if (filename.endsWith(".gif")) {
    return "image/gif";
  } else if (filename.endsWith(".jpg")) {
    return "image/jpeg";
  } else if (filename.endsWith(".ico")) {
    return "image/x-icon";
  } else if (filename.endsWith(".xml")) {
    return "text/xml";
  } else if (filename.endsWith(".pdf")) {
    return "application/x-pdf";
  } else if (filename.endsWith(".zip")) {
    return "application/x-zip";
  } else if (filename.endsWith(".gz")) {
    return "application/x-gzip";
  }
  return "text/plain";
}

bool handleFileRead(String path) {
  DBG_OUTPUT_PORT.println("handleFileRead: " + path);
  if (path.endsWith("/")) {
    path += "index.html";
  }
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz)) {
      path += ".gz";
    }
    File file = SPIFFS.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}




/*
void handleFileUpload() {
  if (server.uri() != "/edit") {
    return;
  }
  HTTPUpload& upload = server.upload();
  if (upload.status == UPLOAD_FILE_START) {
    String filename = upload.filename;
    if (!filename.startsWith("/")) {
      filename = "/" + filename;
    }
    DBG_OUTPUT_PORT.print("handleFileUpload Name: "); DBG_OUTPUT_PORT.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    //DBG_OUTPUT_PORT.print("handleFileUpload Data: "); DBG_OUTPUT_PORT.println(upload.currentSize);
    if (fsUploadFile) {
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {
      fsUploadFile.close();
    }
    DBG_OUTPUT_PORT.print("handleFileUpload Size: "); DBG_OUTPUT_PORT.println(upload.totalSize);
  }
}

void handleFileDelete() {
  if (server.args() == 0) {
    return server.send(500, "text/plain", "BAD ARGS");
  }
  String path = server.arg(0);
  DBG_OUTPUT_PORT.println("handleFileDelete: " + path);
  if (path == "/") {
    return server.send(500, "text/plain", "BAD PATH");
  }
  if (!SPIFFS.exists(path)) {
    return server.send(404, "text/plain", "FileNotFound");
  }
  SPIFFS.remove(path);
  server.send(200, "text/plain", "");
  path = String();
}

void handleFileCreate() {
  if (server.args() == 0) {
    return server.send(500, "text/plain", "BAD ARGS");
  }
  String path = server.arg(0);
  DBG_OUTPUT_PORT.println("handleFileCreate: " + path);
  if (path == "/") {
    return server.send(500, "text/plain", "BAD PATH");
  }
  if (SPIFFS.exists(path)) {
    return server.send(500, "text/plain", "FILE EXISTS");
  }
  File file = SPIFFS.open(path, "w");
  if (file) {
    file.close();
  } else {
    return server.send(500, "text/plain", "CREATE FAILED");
  }
  server.send(200, "text/plain", "");
  path = String();
}

*/


///////////////////////////////////////////////////////
//GLOBAL STATE FOR DETECTOR CARS
String stateDetector;

//GLOBAL SPEED TROTINETTE , INITIALIZE TO 0 
String speed="11";



void handleWarning() {

  String warning;
    if ( server.hasArg("warning") ) {
        warning = server.arg(0);

        DBG_OUTPUT_PORT.printf("warning :%s" , warning.c_str());
        //INSTRUCTIONS : WARNING A METTRE ICI

        if(warning=="0"){
        //on fais clignotter
          Serial.println("WARNING ON <->");
          action = 3;
        }
        if(warning=="1"){
         //on arrete
         Serial.println("WARNING STOP");
         action = 0;
        }
        
      
    } else {
        Serial.println("Bad URL.");
        server.send(404, "text/plain", "Bad URL.");
        return;
    }

}




void handleLeftBlinking() {
 String leftBlinking;
    if ( server.hasArg("leftblinking") ) {
        leftBlinking = server.arg(0);
        DBG_OUTPUT_PORT.printf("<- leftBlinking : %s" , leftBlinking.c_str());


        //INSTRUCTIONS : WARNING A METTRE ICI

        if(leftBlinking=="0"){
        //on fais clignotter
          Serial.println("LIGHT <-");
          action = 1;
        }
        if(leftBlinking=="1"){
         //on arrete
         Serial.println("LIGHT STOP");
         action = 0;
        }
        
      
    } else {
        Serial.println("Bad URL.");
        server.send(404, "text/plain", "Bad URL.");
        return;
    }

}



void handleRightBlinking() {
    String rightBlinking;
    if ( server.hasArg("rightblinking") ) {
        rightBlinking = server.arg(0);
        DBG_OUTPUT_PORT.printf("rightBlinking : %s" ,rightBlinking.c_str());

        //INSTRUCTIONS : WARNING A METTRE ICI

        if(rightBlinking=="0"){
        //on fais clignotter
          action = 2;
          Serial.print("LIGHT ->");
        }
        if(rightBlinking=="1"){
         //on arrete
         Serial.println("LIGHT STOP");
         action = 0;
        }
        
      
    } else {
        Serial.println("Bad URL.");
        server.send(404, "text/plain", "Bad URL.");
        return;
    }
}




void handleSpeed() {

    if ( server.hasArg("speed") ) {

        //UPDATE SPEED      
        speed = server.arg(0);
        DBG_OUTPUT_PORT.printf("speed : %s" , speed.c_str());
     
    } else {
        Serial.println("Bad URL.");
        server.send(404, "text/plain", "Bad URL.");
        return;
    }

}


//RETURN STATE DETECTOR RESPONSE
void handleDistance() {
   Serial.println("State Dector response");
   server.send(200, "text/plain", stateDetector);
   return;
  
}

////////////////////////////////////////////////////////


/*
void handleFileList() {
  if (!server.hasArg("dir")) {
    server.send(500, "text/plain", "BAD ARGS");
    return;
  }


  String path = server.arg("dir");
  DBG_OUTPUT_PORT.println("handleFileList: " + path);
  Dir dir = SPIFFS.openDir(path);
  path = String();

  String output = "[";
  while (dir.next()) {
    File entry = dir.openFile("r");
    if (output != "[") {
      output += ',';
    }
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir) ? "dir" : "file";
    output += "\",\"name\":\"";
    output += String(entry.name()).substring(1);
    output += "\"}";
    entry.close();
  }

  output += "]";
  server.send(200, "text/json", output);
}
*/
void setup(void) {
  DBG_OUTPUT_PORT.begin(115200);
  DBG_OUTPUT_PORT.print("\n");
  DBG_OUTPUT_PORT.setDebugOutput(true);
  SPIFFS.begin();
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      DBG_OUTPUT_PORT.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    DBG_OUTPUT_PORT.printf("\n");
  }


  //WIFI INIT
  DBG_OUTPUT_PORT.printf("Connecting to %s\n", ssid);
  if (String(WiFi.SSID()) != String(ssid)) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    DBG_OUTPUT_PORT.print(".");
  }
  DBG_OUTPUT_PORT.println("");
  DBG_OUTPUT_PORT.print("Connected! IP address: ");
  DBG_OUTPUT_PORT.println(WiFi.localIP());

  MDNS.begin(host);
  DBG_OUTPUT_PORT.print("Open http://");
  DBG_OUTPUT_PORT.print(host);
  DBG_OUTPUT_PORT.println(".local/edit to see the file browser");



/////////////////////////////////////////////////////////////////////////////////////////
  //SERVER INIT

  //activate/ desactivate warning
  server.on("/warning", HTTP_GET, handleWarning);


  //activate/ desactivate rightblinking
  server.on("/rightblinking", HTTP_GET, handleRightBlinking);


  //activate/ desactivate leftblinking
  server.on("/leftblinking", HTTP_GET, handleLeftBlinking);


  //update speed for cars distance 
  server.on("/carsdistance", HTTP_GET, handleSpeed);


  //verify cars distance (thanks to the previous route to have speed)
  //we activate car detector only if speed is > 10  
  server.on("/carsdistance", HTTP_GET, handleDistance);


////////////////////////////////////////////////////////////////////////////////////////////
   
  //list directory
  //server.on("/list", HTTP_GET, handleFileList);
  //load editor
  //server.on("/edit", HTTP_GET, []() {
  //  if (!handleFileRead("/edit.htm")) {
  //    server.send(404, "text/plain", "FileNotFound");
  //  }
  //});
  //create file
  //server.on("/edit", HTTP_PUT, handleFileCreate);
  //delete file
  //server.on("/edit", HTTP_DELETE, handleFileDelete);
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  //server.on("/edit", HTTP_POST, []() {
  //  server.send(200, "text/plain", "");
  //}, handleFileUpload);

  //called when the url is not defined here
  //use it to load content from SPIFFS
  server.onNotFound([]() {
    if (!handleFileRead(server.uri())) {
      server.send(404, "text/plain", "FileNotFound");
    }
  });

  //get heap status, analog input value and all GPIO statuses in one json call
  /*server.on("/all", HTTP_GET, []() {
    String json = "{";
    json += "\"heap\":" + String(ESP.getFreeHeap());
    json += ", \"analog\":" + String(analogRead(A0));
    json += ", \"gpio\":" + String((uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16)));
    json += "}";
    server.send(200, "text/json", json);
    json = String();
  });*/

  // ===================================== TURNING INDICATORS INIT
  strip.begin();
  strip.show(); // Initialize all leds to 'off'
  // ===================================== BLIND SPOT INIT
  pinMode(D2, OUTPUT); // TRIGGER_PIN
  digitalWrite(D2, LOW); // TRIGGER_PIN should be LOW when not using
  pinMode(D1, INPUT); // ECHO_PIN
  // =====================================
  
  server.begin();
  DBG_OUTPUT_PORT.println("HTTP server started");

}






//////////////////////////////////////////////////////////////////////


     

void loop(void) {

  // Check the speed of the vehicle
  if( speed.toInt() > 10 ){
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
    Serial.println(distance_cm);
    if (distance_cm)
    {
      // shifting history of CM values
      for(int i = 0;i<HistorySize;i++)
      {
        if(!valuesHistory[(HistorySize-2)-i]) valuesHistory[(HistorySize-1)-i] = distance_cm;
        valuesHistory[(HistorySize-1)-i] = valuesHistory[(HistorySize-2)-i];
      }
      valuesHistory[0] = distance_cm;
      float med = QuickMedian<float>::GetMedian(valuesHistory, valuesHistoryLen);
      //Serial.print(med); // CM DISTANCE MEDIAN
      //Serial.print(" ");
      float moy = 0;
      for(int i=0;i<HistorySize;i++){ moy += valuesHistory[i]; }
      moy = moy / HistorySize;
      //Serial.print(moy); // CM DISTANCE AVERAGE
      //Serial.print(" ");
      //Serial.println(distance_cm, 2); // CM DISTANCE
      if ( med < 120 )
      {
        Serial.print("CAR DETECTED!! (");
        Serial.print(med);
        Serial.println(")");
        stateDetector = "1"; // DETECTED
      } 
      else 
      {
        Serial.print("NOTHING!! ("); // TEMP
        Serial.print(med);
        Serial.println(")");
        stateDetector = "0"; // NOTHING
      }
    } else stateDetector = "0"; // NOTHING
  } else stateDetector = "0"; // NOTHING
                  
  server.handleClient();
  MDNS.update();

  // ===================================== TURN LIGHT INDICATION
  if (delaycount == 10) delaycount = 0;
  if (delaycount == 0)
  { 
    if (blink == true)
    { 
      Serial.print("BLINKING. action=");
      Serial.println(action);
      if ( action == 1 ) turnLeft();
      if ( action == 2 ) turnRight();
      if ( action == 3 ) warnings();
      if ( stateDetector == "1" ) Break();
      blink = false;
    }
    else
    {
      turnOff();
      /*if ( distance_cm >= 120 ) {
        turnOff();
      }*/
      blink = true;
    }
  }

  // ===================================== COUNTERS
  delaycount++;
  delaylight++;

  delay(40);
}
