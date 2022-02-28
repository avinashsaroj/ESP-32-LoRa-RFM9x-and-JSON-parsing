#include <SPI.h>
#include <LoRa.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

/*#include <SPI.h>
#define BME_SCK 18
#define BME_MISO 19
#define BME_MOSI 23
#define BME_CS 5*/

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BMP280 bme; // I2C
//Adafruit_BME280 bme(BME_CS); // hardware SPI
//Adafruit_BME280 bme(BME_CS, BME_MOSI, BME_MISO, BME_SCK); // software SPI

unsigned long delayTime;

//define the pins used by the transceiver module
#define ss 5
#define rst 14
#define dio0 2


int rain=0;
int prevtip = 0;
float checktemp=0;
float checkhumid=0;
int irPin= 01;

void setup() {
  //initialize Serial Monitor
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println(F("BME280 test"));

  bool status;

  // default settings
  // (you can also pass in a Wire library object like &Wire2)
  status = bme.begin(0x76);  
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  Serial.println("-- Default Test --");
  delayTime = 1000;

  Serial.println();
  
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

 
  float temperature = bme.readTemperature();
  float pressure = bme.readPressure(); 
  float altitude = bme.readAltitude(1013.25);
  
  //Serial.print(temperature);
  //Serial.println(" *C");
  //Serial.print(pressure);
  //Serial.println(" Pa");
   // Serial.print(altitude);
  //Serial.println(" m");
 //Serial.println("print json");
    // Reserve memory space for your JSON data
 
//Serial.println("..........................................");
    StaticJsonBuffer<200> jsonBuffer;
  
  // Build your own object tree in memory to store the data you want to send in the request
  JsonObject& root = jsonBuffer.createObject();
  root["temperature"] = temperature;
  root["pressure"] = pressure;
  root["altitude"] = altitude;
  root["rain"] = rain;
  
  //JsonObject& data = root.createNestedObject("data");
  //data.set("temperature", temperature);
  //data.set("humidity", humidity);
 //root["data"][0] = temperature;
 //root["data"][1] = humidity;
  // Generate the JSON string
  root.printTo(Serial);
  Serial.println("\n");
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
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
  delay(5000);
}
