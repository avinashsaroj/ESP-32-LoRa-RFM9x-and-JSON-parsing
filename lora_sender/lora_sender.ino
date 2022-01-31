#include <SPI.h>
#include <LoRa.h>
#include <ArduinoJson.h>
#include "DHT.h"
#define DHTPIN 22
#define DHTTYPE DHT22


//define the pins used by the transceiver module
#define ss 5
#define rst 14
#define dio0 2

DHT dht(DHTPIN, DHTTYPE);
int rain=0;
int prevtip = 0;
float checktemp=0;
float checkhumid=0;
int irPin= 01;

void setup() {
  //initialize Serial Monitor
  Serial.begin(115200);
  dht.begin();
  while (!Serial);
  Serial.println("LoRa Sender");

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
  if (digitalRead(irPin)==LOW){  
    rain = rain + 1; }
   
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity(); 
  //Serial.println(temperature);
  //Serial.println(humidity);
 //Serial.println("print json");
    // Reserve memory space for your JSON data
  StaticJsonBuffer<200> jsonBuffer;
  
  // Build your own object tree in memory to store the data you want to send in the request
  JsonObject& root = jsonBuffer.createObject();
  root["temperature"] = temperature;
  root["humidity"] = humidity;
  root["rain"] = rain;
  
  //JsonObject& data = root.createNestedObject("data");
  //data.set("temperature", temperature);
  //data.set("humidity", humidity);
 //root["data"][0] = temperature;
 //root["data"][1] = humidity;
  // Generate the JSON string
  root.printTo(Serial);
  Serial.println("\n");
  if(isnan(temperature) || isnan(humidity)){
      Serial.println("Failed to read DHT11");
    }
   if((checktemp-temperature >1) || (checkhumid - humidity >1) || (rain - prevtip >= 5)|| (checktemp-temperature <-1) || (checkhumid - humidity <-1))
   {
    checktemp=temperature;
    checkhumid=humidity;
    prevtip = rain;

    
  Serial.print("Sending packet: ");


  //Send LoRa packet to receiver
  LoRa.beginPacket();
  //LoRa.print("temperature");
  //LoRa.print(temperature);
  //LoRa.print("humidity");
  //LoRa.print(humidity);
  //LoRa.print("rain");
  //LoRa.print(rain);
  root.printTo(LoRa);
  LoRa.endPacket();

   }

  delay(5000);
}
