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
#include <ArtnetWifi.h>

// #include "Arrow.h"
// #include "dummypixels.h"
#include "secrets.h" // local variables

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
uint8_t broadcastAddress[] = {0xC0, 0x49, 0xEF, 0xCF, 0xAD, 0xFC}; // C1

// Artnet
ArtnetWifi artnet;
const int startUniverse = 0;
uint16_t previousDataLength = 0;
int frameNo = 0;

// Define variables to store BME280 readings to be sent
uint16_t ledNum;
uint8_t colR;
uint8_t colG;
uint8_t colB;

// // Define variables to store incoming readings
// uint16_t incomingLedNum;
// uint8_t incomingColR;
// uint8_t incomingColG;
// uint8_t incomingColB;

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

// Create a struct_message for Artnet data
struct_message artnetData;
// Create a struct_message to hold incoming data
struct_message incomingReadings;

esp_now_peer_info_t peerInfo;

// connect to wifi – returns true if successful or false if not
boolean connectWifi(void)
{
  boolean state = true;
  int i = 0;

  WiFi.begin(ssid, password);
  WiFi.mode(WIFI_STA); // Wi-Fi Station
  Serial.println("### ARTNET ESP32 ###");
  Serial.println("Connecting to WiFi");

  // Wait for connection
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    if (i > 20)
    {
      state = false;
      break;
    }
    i++;
  }
  if (state)
  {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("");
    Serial.println("Connection failed.");
  }

  return state;
}

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

// // Callback when data is received (ESP_NOW)
// void onDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
// {
//   memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
//   Serial.printf("Bytes received:\t[%i]\n", len);
//   incomingLedNum = incomingReadings.ledNum;
//   incomingColR = incomingReadings.colR;
//   incomingColG = incomingReadings.colG;
//   incomingColB = incomingReadings.colB;
//   Serial.printf("INCOMING LedNum:\t[%u] | R:[%u] | G: [%u] | B: [%u]\n", incomingLedNum, incomingColR, incomingColG, incomingColB);
// }

// Callback when data is received (DMX)
void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t *data)
{
  Serial.printf("onDmxFrame %u %u %u\n", universe, length, sequence);

  // // set brightness of the whole strip
  // if (universe == 15)
  // {
  //   FastLED.setBrightness(data[0]);
  // }

  // read universe and put into the right part of the display buffer
  // using length/3 because 3 values define r/g/b of one pixel
  // => so this is ONE PIXEL from Qlc+
  // for (int dataNo = 0; dataNo < length / 3; dataNo++)
  // {
  //   // int pxNum = dataNo + (universe - startUniverse) * (previousDataLength / 3);
  //   // //Serial.printf("%i + (%u - %u) * (%u / 3) = %i<%i\n", dataNo, universe, startUniverse, previousDataLength, pxNum, pxTotal);

  //   // if (pxNum < pxTotal)
  //   // {
  //   //   setLedValues(pxNum, dataNo, data);
  //   // }

  //   TODO: need artnetData for each ledNum per frame + send it
  //   artnetData.ledNum = ledNum;
  //   artnetData.colR = data[i * 3];
  //   artnetData.colG = data[i * 3 + 1];
  //   artnetData.colB = data[i * 3 + 2];

  //   Serial.printf("OUTGOING LedNum %u:\t [%u] | R:[%u] | G: [%u] | B: [%u]\n", ledNum, data, data[i * 3], data[i * 3 + 1], data[i * 3 + 2]);
  // }
  // Serial.printf("OUTGOING LedNum:\t[%u] | R:[%u] | G: [%u] | B: [%u]\n", artnetData.ledNum, artnetData.colR, artnetData.colG, artnetData.colB);

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *)&data, sizeof(data));
  if (result == ESP_OK)
  {
    Serial.println("Sent with success");
  }
  else
  {
    Serial.println("Error sending the data");
  }

  // previousDataLength = length;
  // FastLED.show();
  // frameNo++;
}

void setLedValues(int pxNum, int dataNo, uint8_t *data)
{
  int16_t thisCount = 0;
  const int16_t *thisRegion;

  // printf("setLedValues #%i \tpxNum: %i | dataNo: %u\n", dataNo, pxNum);

  switch (pxNum)
  {
  // row 1
  case 10:
    thisCount = len_p11_1;
    thisRegion = p11_1;
    break;
  case 11:
    thisCount = len_p12_1;
    thisRegion = p12_1;
    break;
  case 12:
    thisCount = len_p13_1;
    thisRegion = p13_1;
    break;
  case 13:
    thisCount = len_p14_1;
    thisRegion = p14_1;
    break;
  case 14:
    thisCount = len_p15_1;
    thisRegion = p15_1;
    break;

  // row 2
  case 23:
    thisCount = len_p14_2;
    thisRegion = p14_2;
    break;
  case 24:
    thisCount = len_p15_2;
    thisRegion = p15_2;
    break;
  case 25:
    thisCount = len_p16_2;
    thisRegion = p16_2;
    break;

  // row 3
  case 35:
    thisCount = len_p16_3;
    thisRegion = p16_3;
    break;
  case 36:
    thisCount = len_p17_3;
    thisRegion = p17_3;
    break;
  case 37:
    thisCount = len_p18_3;
    thisRegion = p18_3;
    break;

  // row 4
  case 46:
    thisCount = len_p17_4;
    thisRegion = p17_4;
    break;
  case 47:
    thisCount = len_p18_4;
    thisRegion = p18_4;
    break;
  case 48:
    thisCount = len_p19_4;
    thisRegion = p19_4;
    break;

  // row 5
  case 58:
    thisCount = len_p19_5;
    thisRegion = p19_5;
    break;
  case 59:
    thisCount = len_p20_5;
    thisRegion = p20_5;
    break;
  case 60:
    thisCount = len_p21_5;
    thisRegion = p21_5;
    break;

  // row 6
  case 69:
    thisCount = len_p20_6;
    thisRegion = p20_6;
    break;
  case 70:
    thisCount = len_p21_6;
    thisRegion = p21_6;
    break;
  }

  // printf("setting %i LedValues #%i \tpxNum: %i | dataNo: %u\n", thisCount, dataNo, pxNum);
  for (int l = 0; l < thisCount; l++)
  {
    leds[thisRegion[l]] = CRGB(data[dataNo * 3], data[dataNo * 3 + 1], data[dataNo * 3 + 2]);
    // if (l==0) printf("led %i of region %i \tr: %i | g: %i | b: %i\n", l, pxNum, data[dataNo * 3], data[dataNo * 3 + 1], data[dataNo * 3 + 2]);
  }
}

void setup()
{
  Serial.begin(115200);
  connectWifi();

  // // init LEDs
  // FastLED.addLeds<WS2813, DATA_PIN, GRB>(leds, NUM_LEDS);
  // // FastLED.setBrightness(100);
  // FastLED.setBrightness(255);

  // onDmxFrame will execute every time a packet is received by the ESP32
  artnet.begin();
  artnet.setArtDmxCallback(onDmxFrame);

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
  // // Register for a callback function that will be called when data is received
  // esp_now_register_recv_cb(onDataRecv);
}

void loop()
{
  artnet.read();
}
