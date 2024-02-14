# ESP32-Eagle
<img src="https://asayami.github.io/embedded_source/esp32_deagle_icon.png" style="width:20%">
ESP32 Security Camera (motion detect, send notification, web streaming &amp; configuration)

### Introduction
Main features:
 - Motion Detect
 - Send Images & Notification (Telegram Bot)
 - Web Streaming & Camera Configuration

Vietnamese post: [ESP32-Eagle](https://asayami.github.io/embedded/esp32_cam.html "ESP32-Eagle")
### Firmware
Regarding software, my code is built based on the EloquentSurveillance library. EloquentSurveillance contains individual camera features such as motion detect, web streaming, and send notification. The code of ESP32-Eagle integrates and synchronizes those 3 features to run in parallel, and adds a camera manager (ESP32-Eagle System Manager).

### Installation
To install, read and modify the information in the .ino file as follows:
- Wifi name and password
- Telegram bot info
- Camera type, resolution, clarity, motion detection parameters

Then just load the code into the ESP32 Cam and a message will be sent to your Telegram account about the IP address of the web server.

About how to create a bot and get the code, read [here](https://eloquentarduino.com/esp32-cam-motion-detection/ "here") at the bottom of the page.

Then you can publicize the server by going to the default gateway to foward that IP.

Note: if the esp32 cam connection has a serial port error, go to the device manager port section and uninstall the esp32 driver (must check the box to delete all)
