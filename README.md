# SnipDrop
This repo contains the ESP32 code for my LED rollup banner project 'SnipSign 2.0'. Is uses Expressif's ESPNOW standard to directly communicate between several ESP32s.

## Sources
Getting started with ESPNOW: https://randomnerdtutorials.com/esp-now-two-way-communication-esp32/
Receive data from multiple boards: https://randomnerdtutorials.com/esp-now-many-to-one-esp32/

Tutorial without sensors + several receivers: https://www.instructables.com/ESP32-With-ESP-Now-Protocol/

Artnet on DmxFrame(): https://learn.sparkfun.com/tutorials/using-artnet-dmx-and-the-esp32-to-drive-pixels/all

## Installation
Copy secrets.h_template and rename to secrets.h, fill in SSID and Password for ESP32 Wi-Fi.

## My devices

### MAC addresses
* AP => 40:22:D8:5F:D7:DC
* C1 => C0:49:EF:CF:AD:FC