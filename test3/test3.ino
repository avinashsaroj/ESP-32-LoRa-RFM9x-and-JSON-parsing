#include <WiFi.h>
#include "DHT.h"
#define DHTPIN 33
#define DHTTYPE DHT22
#include <M5Atom.h>


DHT dht(DHTPIN, DHTTYPE);

const char *ssid =  "DJ";     // replace with your wifi ssid and wpa2 key
const char *pass =  "Avinash1!";
const char* host = "192.168.0.103";
int count=0;
int rain=0;
float checktemp=0;
float checkhumid=0;
int irPin= 23;
String Rain(){
   if (digitalRead(irPin)==HIGH){  
    count++; }
   else{
    count=count;
  }
  
  return String(count);
}
void setup()
{
  
    Serial.begin(115200);
    Serial.println("DHT11 Output!");
    dht.begin();
    M5.begin();
    pinMode(irPin, INPUT);

    // We start by connecting to a WiFi network

    Serial.println();
    Serial.println();
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
}



void loop()
{
  
  if (digitalRead(irPin)==LOW){  
    rain = rain + 1; }
   
float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();  
  if(checktemp!=temperature || checkhumid!= humidity && prevtip-currtip!=5){
    if(isnan(temperature) || isnan(humidity)){
      Serial.println("Failed to read DHT11");
    }else{
      checktemp=temperature;
      checkhumid=humidity;
      Serial.print("Humidity: ");
      Serial.print(humidity);
      Serial.print(" %\t");
      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.println(" *C");
      Serial.print("Rain: ");
      Serial.print(rain);
    
    //delay(10000);
  }
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
 client.print(String("GET http://localhost/testcode/dht.php?") + 
                          ("&temperature=") + temperature +
                          ("&humidity=") + humidity +
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
