# Xiaomi M365 SecureBOX
<p align="center">
  <img src="ressources/m365box-gif-md.gif?raw=true">
</p>

## Introduction
Turn signal and blind spot detection for Xiaomi M365

## Hardware requirements
- Weemos D1 Mini (ESP8266, Arduino-compatible layout, wifi, 80/160Mhz, 4Mb flash)<BR>
<p align="center">
  <img src="ressources/wemos.jpg?raw=true">
</p>

- Led strip composed of 14 leds (5V)

- Sonar HC-SR04 or DYP-ME007V1 (5V)
<p align="center">
  <img src="ressources/sonar.jpg?raw=true" style="max-width:50%;">
</p>

- The  Android application, used to control remotly the electric scooter
<p align="center">
  <img src="ressources/appli icone.png?raw=true" style="max-width:50%;">
</p>
The application connect to the Xiaomi M365 with the bluetooth protocol.<br>
This allow the Weemos (the web server) to retreive the speed of the electric scooter.<br>
Here is a screenshot with the interface with the blinking controls, the electric scooter battery and the speed in real time :<br>
<br>
<p align="center">
  <img src="ressources/Screenshot_20190503-045751.jpg?raw=true" style="max-width:50%;">
</p>

## Software requirements

[Arduino IDE with ESP8266 platform installed](https://www.arduino.cc/en/main/software)

If you use windows / OSx you will probably need drivers: [Wemos Driver](https://www.wemos.cc/downloads)

### Library 
- Adafruit_NeoPixel (for the leds)
- ESP8266WiFi (for the Wifi)
- QuickMedianLib (for the median filter)

### External library
- ESP8266FS (SPIFFS, see: [Using ESP8266 SPIFFS tutorial](https://www.instructables.com/id/Using-ESP8266-SPIFFS/) )

## How does it work ?
<p align="center">
  <img src="ressources/schema1.jpg?raw=true" style="max-width:50%;">
</p>
The electric scooter Xiaomi M365 have a bluetooth connection that can be used to get some informations like the actual speed, the battery. This is sent to the android application.<br>
The Weemos is connected to the phone wifi hotspot, and so the can connect it via the android application from a webview. The android application is a bridge from the bluetooth to the web interface.
<br>
<p align="center">
  <img src="ressources/schema2.jpg?raw=true" style="max-width:50%;">
</p>

### Wiring
Firstly, the Arduino card connect on a WiFi network. Modify the SSID and Password at the top of the code.<br>
Secondly, use the diagram bellow the connect the sonar (enabling blind spot detection) and the led strip (enabling turn signal and warning signal) <br>

![Conneciton schema](ressources/ConnectionSchema.png?raw=true)
You can found the original file in `ressources/Connection.fzz`

## Issues

### The sonar HC-SR04 / DYP-ME007V1 filtering
<br>
While using the sonar HC-SR04 or DYP-ME007V1 we had problems with the echo signal. The original signal had random peaks (green signal on the picture bellow), like noise. We do not want this peaks as it is generating a bad blind spot detection randomly, even when we put a condition at 120CM or less to engage the blind spot signal.<br>
<br>
To solve this problem, we put 2 additionnal signals :<br>
-The median of the 15 last values of the original signal (blue)<br>
-The moving average of the 15 last values of the original signal (red)<br>
<br>

![Problem sonar](ressources/problem.jpg?raw=true)

<br>
As we can see on the picture above, an array of 15 values is not enought for making the signal flat and removing the noise we do not want. Let's try with 30 values :<br>
<br>

![Problem sonar 2](ressources/problem2.jpg?raw=true)

<br>
While trying to simulate an obstacle, we saw that it was slow to detect the signal, almost 2/3 of the real signal (green) is lost because of the median filter if we use it.<br>
We have been searching the perfect value and we have found that 20 is a good value :<br>
<br>

![Problem sonar 3](ressources/problem3.jpg?raw=true)

<br>
Both the moving average and the median are removing most of the peaks that we do not want here. As the median is more "flat", we used this signal to solve the problem.<br>

# Building the SecureBox

<p align="center">
  <img src="ressources/schema-construction.jpg?raw=true" style="max-width:50%;">
</p>

## Video of the prototype

[Click here to see the video](https://drive.google.com/file/d/1nNjBMYanGiluA92XC430YwJ56WK5rWbO/view?usp=sharing)