#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
#include <Timer.h>

Timer timer;


const String device = String("{") +
                                                              "\"deviceType\":\"lightingControl\","      +
                                                              "\"deviceCode\": \"lt01\","                           +
                                                               "\"numberOfPorts\": 2 ,"                               +
                                                               "\"lights\": ["                                                      +
                                                                    "{"                                                                     +
                                                                        "\"typeOfControl\": \"DIM\","            +
                                                                   " },"                                                                    +
                                                                    "{"                                                                     +
                                                                        "\"typeOfControl\": \"ON/OFF\","   +
                                                                   " }"                                                                     +                                                                   
                                                           " ]"                                                                             +
                                                          "}";

const char* ssid     = "BKHome";
const char* password = "bkhomebkhome";

//khai bao mqtt
#define host "192.168.1.102"
#define mqtt_topic_pub "presence"
#define mqtt_topic_sub "presence"
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
   //Serial.begin(9600);
  delay(10);
  
  //Serial1.begin(115200);
  Serial1.setDebugOutput(true);
  delay(10);

  Serial1.println();
  Serial1.println();
  Serial1.print("Connecting to ");
  Serial1.println(ssid);
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial1.print(".");
    delay(500);

  }

  //set up mqtt
  client.setServer(host, mqtt_port);
  client.setCallback(callback);

  Serial1.println("");
  Serial1.println("WiFi connected");  
  Serial1.println("IP address: ");
  Serial1.println(WiFi.localIP());

  timer.every(1, lightingControl);
  //timer.every(1000, authenticate);
}

void lightingControl(){
  if(digitalRead(pinin)){
    while(digitalRead(pinin));
      delayMicroseconds(delayTime);
      digitalWrite(pinout,HIGH);
      delayMicroseconds(10);
      digitalWrite(pinout,LOW); 
  }

  digitalWrite(led, ledValue);
}

//Authenticate
void authenticate(){

        HTTPClient http;

        String url  ="http://" + (String)host + ":" + (String)http_port + "/devices/test";
        Serial1.println("[HTTP] begin...");
        http.begin(url); //HTTP
    
        Serial1.println("[HTTP] POST:" + url);
        http.addHeader("Content-Type", "application/json");
        http.POST("{\"test\":\"testesafdasdf\"}");
        String payload = http.getString();

        Serial1.println(payload);

        http.end();
}

//mqtt
void callback(char* topic, byte* payload, unsigned int length) {
  String rmsg = "";
  //Serial1.print("Message arrived: [");
  //Serial1.print(topic);
  //Serial1.print("] ");
  for (int i = 0; i < length; i++) {
    //Serial1.print((char)payload[i]);
    rmsg += (char)payload[i];
  }
  delayTime = rmsg.toInt();
  Serial1.println(rmsg);
}

//mqtt
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial1.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial1.println("connected");
      // Once connected, publish an announcement...
      client.publish(mqtt_topic_pub, "ESP_reconnected");
      // ... and resubscribe
      client.subscribe(mqtt_topic_sub);
    } else {
      Serial1.print("failed, rc=");
      Serial1.print(client.state());
      Serial1.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}



void loop(){
  //Serial1.println(product_info);
   //mqtt
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  timer.update();
}

