#include <Arduino.h>

#include <ESP8266WiFi.h>
//#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
#include <Timer.h>
//#include <ArduinoJson.h>


#define sw1 9
#define sw2 10
#define light4 16
#define light2 4
#define light3 5

Timer timer;
int authenticate_timer;

const char* device = "{\"deviceType\":\"LightingControl\",\"deviceCode\": \"lt04\",\"numberOfPorts\": 4, \"lights\": [{\"portId\": 1, \"dimmable\": true},{\"portId\": 2, \"dimmable\":  false},{\"portId\": 3, \"dimmable\":  false}]}";
String device_id = "";


const char* ssid     = "BKHome";
const char* password = "bkhomebkhome";

//khai bao mqtt
#define host "192.168.10.100"
#define mqtt_topic_pub "presence"
#define mqtt_topic_sub "presence"
#define mqtt_topic_lwt "lwt"
#define mqtt_user "alice"
#define mqtt_pwd "secret"
const uint16_t mqtt_port = 1883;
const uint16_t http_port = 3000;


boolean s2 = false,  s3 = false, s4 = false;
boolean s2_previous = false, s3_previous = false, s4_previous = false;

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
  Serial.println("Authenticating lightingcontrol...");
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
    device_id = rmsg;
    String topic = "devices/" + device_id;
    client.subscribe(topic.c_str());
    Serial.println(topic.c_str());
  }  else {
    char light = rmsg.charAt(0);
    boolean *s, *s_previous;
    switch(light){
      case '1':
        Serial.println(rmsg);
        break;
      case '2':
        s = &s2;
        s_previous = &s2_previous;  
        *s = (rmsg.charAt(1) - '0')==1;
        if(*s!=*s_previous) {
  //        *s_previous = *s;
          Serial.println(rmsg);
  //        delay(100);
        }
        break;
     case '3':
        s = &s3;
        s_previous = &s3_previous;
        *s = (rmsg.charAt(1) - '0')==1;
        if(*s!=*s_previous) {
  //        *s_previous = *s;
          Serial.println(rmsg);
  //        delay(100);
        }
        break;
     case '4':
        s = &s4;
        s_previous = &s4_previous;
        *s = (rmsg.charAt(1) - '0')==1;
        if(*s!=*s_previous) {
  //        *s_previous = *s;
          Serial.println(rmsg);
  //        delay(100);
        }
        break;
    }
  }
}

// Connect to mqtt broker
void checkMqttConnection() {
  // Loop until we're reconnected
  if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("lt04", "lwt", 0, false, "lt04")) {
      Serial.println("connected");
      //client.publish("connected", "hello");\
      delay(10);
      client.subscribe("authenticate/lt04");
      authenticate_timer = timer.every(5000, authenticate);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 3 seconds");
    }
  }
}

void readButtons(){
  if(!digitalRead(sw1)){
    while(!digitalRead(sw1));
    Serial.println("00");
  }
  if(!digitalRead(sw2)){
    while(!digitalRead(sw2));
    Serial.println("01");
  }
}

void readStatusLight(int port){
  int light;
  boolean *stt, *stt_previous;
  switch(port){
    case 2: 
        light = light2;
        stt  = &s2;
        stt_previous = &s2_previous;
        break;
     case 3: 
        light = light3;
        stt  = &s3;
        stt_previous = &s3_previous;
        break;
     case 4: 
        light = light4;
        stt  = &s4;
        stt_previous = &s4_previous;
        break;
  }
   int t = millis();
    *stt = false;
    while((millis() - t)<10){
      if(!digitalRead(light)){
        *stt = true;
      }
    }
    
    if(*stt!=*stt_previous){
        *stt_previous = *stt;
        String topic = "devices/lighting-control/" + device_id + "/" + port;
        char msg[10];
        sprintf(msg, "%d",  *stt);
        client.publish(topic.c_str(), msg);
    }
}

void readLightsStatus(){
    readStatusLight(2);
    readStatusLight(3);
    readStatusLight(4);
}

void sw1_Interrupt(){
  Serial.println("00");
}

void sw2_Interrupt(){
  Serial.println("01");
}

void setup() {
//    pinMode(sw1, INPUT);
//    pinMode(sw2, INPUT);
    pinMode(sw1, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(sw1), sw1_Interrupt, FALLING);
 
    pinMode(sw2, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(sw2), sw2_Interrupt, FALLING);
   

    pinMode(light2, INPUT);
    pinMode(light3, INPUT);
    pinMode(light4, INPUT);
    
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
  timer.every(3000, checkMqttConnection);
}


void loop(){
  client.loop();
  timer.update();
//  readButtons();
  readLightsStatus();
}
