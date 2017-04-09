/*
 *  This sketch sends data via HTTP GET requests to data.sparkfun.com service.
 *
 *  You need to get streamId and privateKey at data.sparkfun.com and paste them
 *  below. Or just customize this script to talk to other HTTP servers.
 *
 */

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

const char* ssid     = "BKHome";
const char* password = "bkhomebkhome";

void setup() {
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void loop() {
    // wait for WiFi connection

        HTTPClient http;

        Serial.print("[HTTP] begin...\n");
        http.begin("http://192.168.43.201:3000/devices/test"); //HTTP
    
        Serial.print("[HTTP] POST...\n");
        http.addHeader("Content-Type", "application/json");
        http.POST("{\"test\":\"testesafdasdf\"}");
        String payload = http.getString();
                Serial.println(payload);

        http.end();
    

    delay(10000);
}

