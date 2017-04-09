#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
#include <Timer.h>
#include <ArduinoJson.h>

Timer timer;
int authenticate_timer;


const char* device = "{\"deviceType\":\"LightingControl\",\"deviceCode\": \"lt04\",\"numberOfPorts\": 2, \"lights\": [{\"portId\": 1, \"dimmable\": true},{\"portId\": 2, \"dimmable\":  false}]}";
String device_Id = "";

const char* ssid     = "BKHome";
const char* password = "bkhomebkhome";

//khai bao mqtt
#define host "192.168.10.102"
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



//Authenticate
void authenticate(){
   client.publish("authenticate", device);
}

//mqtt callback function
void callback(char* topic, byte* payload, unsigned int length) {
  String rmsg = "";
//  Serial.print("Message arrived: [");
//  Serial.print(topic);
//  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    //Serial.print((char)payload[i]);
    rmsg += (char)payload[i];
  }

  if((String)topic == "authenticate/lt04"){
    timer.stop(authenticate_timer);
    device_Id = rmsg;
    String topic = "devices/" + device_Id;
    client.subscribe(topic.c_str());
    Serial.println(topic.c_str());
  }

  delayTime = rmsg.toInt();
  Serial.println(rmsg);
}

// Connect to mqtt broker
void checkMqttConnection() {
  // Loop until we're reconnected
  if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("lt04", "lwt", 0, false, "lt04")) {
      authenticate_timer = timer.every(5000, authenticate);
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("connected", "hello");
      // ... and resubscribe
      client.subscribe("authenticate/lt04");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
    }
  }
}

void setup() {
    pinMode(pinout, OUTPUT);
    pinMode(pinin, INPUT);
    pinMode(led, OUTPUT);
   Serial.begin(9600);
   delay(10);

  //Serial.begin(115200);
  //Serial.setDebugOutput(true);
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
}

void loop(){
  client.loop();
  timer.update();
}
