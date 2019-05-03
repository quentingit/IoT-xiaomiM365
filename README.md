# Xiaomi M365 SecureBOX
<p align="center">
  <img src="ressources/m365box-gif-md.gif?raw=true">
</p>

## Introduction
Turn signal and blind spot detection for Xiaomi M365

## Hardware requirements
- Weemos D1 Mini (ESP8266, Arduino-compatible layout, wifi, 80/160Mhz, 4Mb flash)<BR>
![Weemos D1 Mini](ressources/wemos.jpg?raw=true){ style="display: block; margin: 0 auto" }
  
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

Firstly, the Arduino card connect on a WiFi network. Modify the SSID and Password at the top of the code.<br>
Secondly, use the diagram bellow the connect the sonar (enabling blind spot detection) and the led strip (enabling turn signal and warning signal) <br>

![Conneciton schema](ressources/ConnectionSchema.png?raw=true)

## Issues

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

