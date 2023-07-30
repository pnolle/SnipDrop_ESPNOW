# SnipDrop attempt with ESPNOW
This repo contains ESP32 code developed for my LED rollup banner project 'SnipDrop' aka. 'SnipSign 2.0'. It uses Expressif's ESPNOW standard to directly communicate between several ESP32s. 

## Reason for deprecation
I ditched this attempt because I found ESPNOW tutorials on how to receive data from multiple senders, but I need to receive data from one sender with multiple receivers. It just seemed easier to go ahead with the 'classic' approach of setting up a Wi-Fi access point on one ESP32 and connecting two others two it and broadcasting the lighting information from Qlc+.
=> This is the project I went live with: https://github.com/pnolle/SnipDrop_Esp32Wifi

## Motivaton
http://snippetupperlaser.com

## Sources
Getting started with ESPNOW: https://randomnerdtutorials.com/esp-now-two-way-communication-esp32/
Receive data from multiple boards: https://randomnerdtutorials.com/esp-now-many-to-one-esp32/

Tutorial without sensors + several receivers: https://www.instructables.com/ESP32-With-ESP-Now-Protocol/

## Installation
Copy secrets.h_template and rename to secrets.h, fill in SSID and Password for ESP32 Wi-Fi.

## My devices

### MAC addresses
* AP => 40:22:D8:5F:D7:DC
* C1 => C0:49:EF:CF:AD:FC