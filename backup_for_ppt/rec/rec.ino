#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <ArduinoJson.h>

//define the pins used by the transceiver module
#define ss 5
#define rst 14
#define dio0 2
const char *ssid =  "DJ";     // replace with your wifi ssid and wpa2 key
const char *pass =  "Avinash1!";
const char* host = "192.168.0.104";
int rain=0;
float temperature =0;
float pressure =0;

void setup() {
  //initialize Serial Monitor
  Serial.begin(115200);

    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(ssid, pass);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  
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
    float latitude = root["temperature"];
    float longitude = root["pressure"];
    int rain = root["rain"];
    Serial.println("parsing \n");
    Serial.println(latitude);
    Serial.println(longitude);
    temperature = latitude;
    pressure = longitude;
    }

    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());


  }

  Serial.print("connecting to ");
    Serial.println(host);

    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    const int httpPort = 80;
    if (!client.connect(host, httpPort)) {
        Serial.println("connection failed to wifi");
        return;
    }

 
//delay(5000);

    // This will send the request to the server
 client.print(String("GET http://localhost/testcode_backup/dht.php?") + 
                          ("&temperature=") + temperature +
                          ("&pressure=") + pressure +
                          ("&rain=") + rain +
                          " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");
    unsigned long timeout = millis();
    while (client.available() == 0) {
        if (millis() - timeout > 11000) {
            Serial.println(">>> Client Timeout !");
            client.stop();
            return;
        }
    }

    // Read all the lines of the reply from server and print them to Serial
    while(client.available()) {
        String line = client.readStringUntil('\r');
        Serial.print(line);
        
    }

    Serial.println();
    Serial.println("closing connection");

     
}
