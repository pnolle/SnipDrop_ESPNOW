/*
  #################
  SnipDrop - sender
  #################

  by Niklas Köhn
  http://snippetupperlaser.com

  This is the firmware for the ESP32 receiving Artnet data from Qlc+ and sending it to the LED controllers via ESPNOW.

  ESP_NOW parts based on Rui Santos' work at https://RandomNerdTutorials.com/esp-now-two-way-communication-esp32/
*/

// #include <FastLED.h>
#include <esp_now.h>
#include <WiFi.h>

// TODO: is there an ArtnetUSB?
// #include <ArtnetWifi.h>

// #include "Arrow.h"
// #include "dummypixels.h"
// #include "secrets.h" // local variables

// // How many leds in your strip?
// #define NUM_LEDS 452 // 452 LEDs in Arrow
// // #define NUM_LEDS 515 // 515 LEDs in Laser v2 + Scissors
// // #define NUM_LEDS 507  // 507 LEDs in Circle

// #define NUM_ROWS 5
// #define NUM_COLS 10

// // For led chips like Neopixels, which have a data line, ground, and power, you just
// // need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// // ground, and power), like the LPD8806, define both DATA_PIN and CLOCK_PIN
// #define DATA_PIN 12
// // #define CLOCK_PIN 13

// // Define the array of leds
// CRGB leds[NUM_LEDS];

// REPLACE WITH THE MAC Address of your receiver
// uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
// uint8_t broadcastAddress[] = {0x40, 0x22, 0xD8, 0x5F, 0xD7, 0xDC};  // AP
// uint8_t broadcastAddress[] = {0xC0, 0x49, 0xEF, 0xCF, 0xAD, 0xFC}; // C1
uint8_t broadcastAddress[] = {0x34, 0x86, 0x5D, 0xFC, 0x80, 0xB4}; // REC

// // Artnet
// ArtnetWifi artnet;
// const int startUniverse = 0;
// uint16_t previousDataLength = 0;
// int frameNo = 0;

// Define variables to store BME280 readings to be sent
uint16_t ledNum;
uint8_t colR;
uint8_t colG;
uint8_t colB;

// Define variables to store incoming readings
uint16_t incomingLedNum;
uint8_t incomingColR;
uint8_t incomingColG;
uint8_t incomingColB;

// Variable to store if sending data was successful
String success;

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message
{
  uint16_t ledNum;
  uint8_t colR;
  uint8_t colG;
  uint8_t colB;
} struct_message;

// Create a struct_message for dummy data
struct_message DummyData;
// Create a struct_message for Artnet data
struct_message artnetData;
// Create a struct_message to hold incoming data
struct_message incomingReadings;

esp_now_peer_info_t peerInfo;

// // connect to wifi – returns true if successful or false if not
// boolean connectWifi(void)
// {
//   boolean state = true;
//   int i = 0;

//   WiFi.begin(ssid, password);
//   WiFi.mode(WIFI_STA); // Wi-Fi Station
//   Serial.println(WiFi.macAddress());

//   // Wait for connection
//   Serial.print("Connecting to Wi-Fi");
//   while (WiFi.status() != WL_CONNECTED)
//   {
//     delay(500);
//     Serial.print(".");
//     if (i > 20)
//     {
//       state = false;
//       break;
//     }
//     i++;
//   }
//   if (state)
//   {
//     Serial.println("");
//     Serial.print("Connected to ");
//     Serial.println(ssid);
//     Serial.print("IP address: ");
//     Serial.println(WiFi.localIP());
//   }
//   else
//   {
//     Serial.println("");
//     Serial.println("Connection failed.");
//   }

//   return state;
// }

// Callback when data is sent (ESP_NOW)
void onDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("Last Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status == 0)
  {
    success = "Delivery Success :)";
  }
  else
  {
    success = "Delivery Fail :(";
  }
}

// Callback when data is received (ESP_NOW)
void onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  Serial.printf("Bytes received:\t[%i]\n", len);
  incomingLedNum = incomingReadings.ledNum;
  incomingColR = incomingReadings.colR;
  incomingColG = incomingReadings.colG;
  incomingColB = incomingReadings.colB;
  Serial.printf("INCOMING LedNum:\t[%u] | R:[%u] | G: [%u] | B: [%u]\n", incomingLedNum, incomingColR, incomingColG, incomingColB);
}

// // Callback when data is received (DMX)
// void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t *data)
// {
//   Serial.printf("onDmxFrame %u %u %u\n", universe, length, sequence);

//   // // set brightness of the whole strip
//   // if (universe == 15)
//   // {
//   //   FastLED.setBrightness(data[0]);
//   // }

//   // read universe and put into the right part of the display buffer
//   // using length/3 because 3 values define r/g/b of one pixel
//   // => so this is ONE PIXEL from Qlc+
//   // for (int dataNo = 0; dataNo < length / 3; dataNo++)
//   // {
//   //   // int pxNum = dataNo + (universe - startUniverse) * (previousDataLength / 3);
//   //   // //Serial.printf("%i + (%u - %u) * (%u / 3) = %i<%i\n", dataNo, universe, startUniverse, previousDataLength, pxNum, pxTotal);

//   //   // if (pxNum < pxTotal)
//   //   // {
//   //   //   setLedValues(pxNum, dataNo, data);
//   //   // }

//   //   TODO: need artnetData for each ledNum per frame + send it
//   //   artnetData.ledNum = ledNum;
//   //   artnetData.colR = data[i * 3];
//   //   artnetData.colG = data[i * 3 + 1];
//   //   artnetData.colB = data[i * 3 + 2];

//   //   Serial.printf("OUTGOING LedNum %u:\t [%u] | R:[%u] | G: [%u] | B: [%u]\n", ledNum, data, data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
//   // }
//   // Serial.printf("OUTGOING LedNum:\t[%u] | R:[%u] | G: [%u] | B: [%u]\n", artnetData.ledNum, artnetData.colR, artnetData.colG, artnetData.colB);

//   // Send message via ESP-NOW
//   esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&data, sizeof(data));
//   if (result == ESP_OK)
//   {
//     Serial.println("Sent with success");
//   }
//   else
//   {
//     Serial.println("Error sending the data");
//   }

//   // previousDataLength = length;
//   // FastLED.show();
//   // frameNo++;
// }

void setup()
{
  Serial.begin(115200);
  Serial.println("### SnipDrop - sender ###");
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  // connectWifi();

  // // init LEDs
  // FastLED.addLeds<WS2813, DATA_PIN, GRB>(leds, NUM_LEDS);
  // // FastLED.setBrightness(100);
  // FastLED.setBrightness(255);

  // // onDmxFrame will execute every time a packet is received by the ESP32
  // artnet.begin();
  // artnet.setArtDmxCallback(onDmxFrame);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK)
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  // register for sending CB to get the status of transmitted packet
  esp_now_register_send_cb(onDataSent);

  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  // Add peer
  if (esp_now_add_peer(&peerInfo) != ESP_OK)
  {
    Serial.println("Failed to add peer");
    return;
  }
  else {
    Serial.printf("Peer added %u\n", broadcastAddress);
  }
  
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(onDataRecv);
}

void loop()
{
  Serial.println("loop");
  // artnet.read();

  generateDummyData();
  DummyData.ledNum = ledNum;
  DummyData.colR = colR;
  DummyData.colG = colG;
  DummyData.colB = colB;

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &DummyData, sizeof(DummyData));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  delay(1000);
}


void generateDummyData(){
  ledNum = random(70);
  colR = random(255);
  colG = random(255);
  colB = random(255);
  Serial.printf("OUTGOING LedNum:\t[%u] | R:[%u] | G: [%u] | B: [%u]\n", ledNum, colR, colG, colB);
}
