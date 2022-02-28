#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include "BluetoothSerial.h"

//define the pins used by the transceiver module
#define ss 5
#define rst 14
#define dio0 2
int LED = 13;
const char *ssid =  "DJ";     // replace with your wifi ssid and wpa2 key
const char *pass =  "Avinash1!";
const char* host = "192.168.0.103";
int rain=0;
float temperature =0;
float humidity =0;

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

void setup() {
  //initialize Serial Monitor
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  SerialBT.begin("ESP32test"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");

  
  while (!Serial);
  Serial.println("LoRa Receiver");

  //setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);
  
  //replace the LoRa.begin(---E-) argument with your location's frequency 
  //433E6 for Asia
  //866E6 for Europe
  //915E6 for North America
  while (!LoRa.begin(866E6)) {
    Serial.println(".");
    delay(500);
  }
   // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");
}

void loop() {
  StaticJsonBuffer<200> jsonBuffer;
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");
    SerialBT.println("Received packet \n");
      digitalWrite(LED, HIGH);
      delay(500);
      digitalWrite(LED, LOW);
      delay(500);

    // read packet
    while (LoRa.available()) {
      String LoRaData = LoRa.readString();
      Serial.print(LoRaData); 

      delay(5000);

    char json[] = "{\"sensor\":\"gps\",\"data\":[\"temperature\":\"48.756080\",\"humidity\":\"2.302038\"]}";
    //json = LoRaData;
    JsonObject& root = jsonBuffer.parseObject(LoRaData);
    if(!root.success()) {
    Serial.println("parseObject() failed");
    //return false;
    }
    double latitude = root["temperature"];
    double longitude = root["humidity"];
    int rain = root["rain"];
    //Serial.println("parsing \n");
    //Serial.println(latitude);
    //Serial.println(longitude);
    //temperature = latitude;
    //humidity = longitude;
    }

    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
    SerialBT.println(LoRa.packetRssi());

  }

 

     
}
