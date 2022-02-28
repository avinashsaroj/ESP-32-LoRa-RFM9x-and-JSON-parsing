#include <SPI.h>
#include <LoRa.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include "BluetoothSerial.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "time.h"

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;

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

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;
int rain=0;
int prevtip = 0;
float checktemp=0;
float checkhumid=0;
int irPin= 01;

String dataMessage;

// NTP server to request epoch time
const char* ntpServer = "pool.ntp.org";

// Variable to save current epoch time
unsigned long epochTime; 

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

// Initialize SD card
void initSDCard(){
   if (!SD.begin()) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No SD card attached");
    return;
  }
  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
    Serial.println("MMC");
  } else if(cardType == CARD_SD){
    Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
}

// Write to the SD card
void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if(file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

// Append data to the SD card
void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if(file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}
const char *filename = "/data.txt"; 
void setup() {
  //initialize Serial Monitor
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  SerialBT.begin("ESP32test"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
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

  
  initSDCard();
  configTime(0, 0, ntpServer);
  
  // If the data.txt file doesn't exist
  // Create a file on the SD card and write the data labels
  File file = SD.open("/data.txt");
  if(!file) {
    Serial.println("File doesn't exist");
    Serial.println("Creating file...");
    writeFile(SD, "/data.txt", "Epoch Time, Meteorological Data  \r\n");
  }
  else {
    Serial.println("File already exists");  
  }
  file.close();
}

void loop() {
  epochTime = getTime();
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


      //Concatenate all info separated by commas
   char   sensorData = root["temperature", "pressure", "altitude"];
  dataMessage = String(epochTime) + "," +sensorData + "\r\n";
  Serial.print("Saving data: ");
  Serial.println(dataMessage);

  //Append the data to file
  appendFile(SD, "/data.txt", dataMessage.c_str());

  lastTime = millis();
  //JsonObject& data = root.createNestedObject("data");
  //data.set("temperature", temperature);
  //data.set("humidity", humidity);
 //root["data"][0] = temperature;
 //root["data"][1] = humidity;
  // Generate the JSON string
  root.printTo(Serial);
  Serial.println("\n");
    Serial.print("Sending packet: ");

  root.printTo(SerialBT);
  
  SerialBT.println(dataMessage);
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
