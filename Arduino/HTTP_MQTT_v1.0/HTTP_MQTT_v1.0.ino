#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
#include <Timer.h>
#include <ArduinoJson.h>

Timer timer;
int timer_http;


const char* device = "{\"deviceType\":\"LightingControl\",\"deviceCode\": \"lt04\",\"numberOfPorts\": 2, \"lights\": [{\"portId\": 1, \"dimmable\": true},{\"portId\": 2, \"dimmable\":  false}]}";


const char* ssid     = "phuongchi11";
const char* password = "20112009";

//khai bao mqtt
#define host "192.168.1.102"
#define mqtt_topic_pub "presence"
#define mqtt_topic_sub "presence"
#define mqtt_topic_lwt "lwt"
#define mqtt_user "alice"
#define mqtt_pwd "secret"
const uint16_t mqtt_port = 1883;
const uint16_t http_port = 3000;


 #define pinout 12
 #define pinin 14
 #define led 13

 int delayTime = 8000;
int light = 0;
int ledValue = 0;


//MQTT
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;


void setup() {
    pinMode(pinout, OUTPUT);
    pinMode(pinin, INPUT);
    pinMode(led, OUTPUT);
   Serial.begin(9600);
   delay(10);
  
  //Serial1.begin(115200);
  //Serial1.setDebugOutput(true);
  delay(10);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);

  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(host, mqtt_port);
  client.setCallback(callback);
  timer.every(5000, checkMqttConnection);
  
 // timer_http = timer.every(5000, authenticate);
}

//Authenticate
void authenticate(){

        HTTPClient http;

        String url  ="http://" + (String)host + ":" + (String)http_port + "/devices/authenticate";
        Serial.println("[HTTP] begin...");
        http.begin(url); //HTTP
    
        Serial.println("[HTTP] POST:" + url);
        http.addHeader("Content-Type", "application/json");
        http.POST(device);
        String res = http.getString();

        Serial.println(res);
        StaticJsonBuffer<200> jsonBuffer;
        JsonObject& root = jsonBuffer.parseObject(res);
        if (!root.success()) {
          Serial.println("parseObject() failed");
          return;
        }
        const boolean allowToConnect = root["allowToConnect"];
       if(allowToConnect){
          timer.stop( timer_http);
          //set up mqtt
          
       } 

        http.end();
}

//mqtt
void callback(char* topic, byte* payload, unsigned int length) {
  String rmsg = "";
  //Serial.print("Message arrived: [");
  //Serial.print(topic);
  //Serial.print("] ");
  for (int i = 0; i < length; i++) {
    //Serial.print((char)payload[i]);
    rmsg += (char)payload[i];
  }
  delayTime = rmsg.toInt();
  Serial.println(rmsg);
}

//mqtt
void checkMqttConnection() {
  // Loop until we're reconnected
  if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("lt04", "lwt", 0, false, "lt04")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("connected", "hello");
      Serial.println(strlen(device));
      Serial.println(5 + 2+strlen("connected") + strlen(device));
      client.publish("connected", device);
      // ... and resubscribe
      client.subscribe("connected");
      client.subscribe("connected/lt04");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
    }
  }
}



void loop(){
  client.loop();
  timer.update();
}

