# Xiaomi M365 SecureBOX
![Xiaomi M365 SecureBOX](ressources/m365box-gif-md.gif?raw=true)

## Introduction
Turn signal and blind spot detection for Xiaomi M365

## Hardware requirements
- Weemos D1 Mini (ESP8266, Arduino-compatible layout, wifi, 80/160Mhz, 4Mb flash)<BR>
![Weemos D1 Mini](ressources/wemos.jpg?raw=true)
  
## Software requirements

[Arduino IDE with ESP8266 platform installed](https://www.arduino.cc/en/main/software)

If you use windows / OSx you will probably need drivers: [Wemos Driver](https://www.wemos.cc/downloads)

## How does it work ?

Firstly, the Arduino card connect on a WiFi network. Modify the SSID and Password at the top of the code.
Secondly, use the diagram bellow the connect the sonar (enabling blind spot detection) and the led strip (enabling turn signal and warning signal) 
![Conneciton schema](ressources/ConnectionSchema.png?raw=true)

## Issues

While using the sonar HC-SR04 or DYP-ME007V1 we had problems with the echo signal. The original signal had random peaks (green signal on the picture bellow), like noise. We do not want this peaks as it is generating a bad blind spot detection randomly, even when we put a condition at 120CM or less to engage the blind spot signal. 

![Problem sonar](ressources/problem.jpg?raw=true)

To solve this problem, we put 2 additionnal signals :
-The median of the 15 last values of the original signal (blue)
-The moving average of the 15 last values of the original signal (red)

As we can see on the picture, both the moving average and the median are removing most of the peaks that we do not want here. As the median is more "flat", we used this signal to solve the problem.