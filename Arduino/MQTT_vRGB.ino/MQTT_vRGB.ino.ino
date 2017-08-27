#include <Arduino.h>

#include <ESP8266WiFi.h>
//#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
#include <Timer.h>
//#include <ArduinoJson.h>

//define GPIO pin
 #define red 13
 #define green 15
 #define blue 16
#define emrgcy_pin 10
#define bzz_pin 5
#define magDoor_pin 12
#define doorSensor_pin 4

Timer timer;
int authenticate_timer;


char* rgb = "{\"deviceType\":\"LightingControl\",\"deviceCode\": \"rgb\",\"numberOfPorts\": 1, \"lights\": [{\"portId\": 1, \"dimmable\": false, \"typeOfLight\": \"RGB\"}]}";
String rgb_id = "";

char* buzzer = "{\"deviceType\":\"LightingControl\", \"deviceCode\": \"bzz\",\"lights\": [{\"portId\": 1,\"name\": \"Buzzer\",\"dimmable\": false,\"typeOfLight\": \"Alarm\"}]}";
String buzzer_id = "";

char* emrgcy = "{\"deviceType\":\"SensorModule\",\"deviceCode\": \"emrgcy\",\"sensors\": [{\"name\": \"Emergency\", \"_type\": \"Emergency\", \"value\": 0}]}";
String emrgcy_id = "";

char* door = "{\"deviceType\":\"SensorModule\",\"deviceCode\": \"dss\",\"sensors\": [{\"name\": \"Door\", \"_type\": \"Door\", \"value\": 0}]}";
String door_id = "";

const char* ssid     = "BKHome";
const char* password = "bkhomebkhome";

//define mqtt parameters
#define host "192.168.10.100"
#define mqtt_topic_pub "presence"
#define mqtt_topic_sub "presence"
#define mqtt_topic_lwt "lwt"
#define mqtt_user "alice"
#define mqtt_pwd "secret"
const uint16_t mqtt_port = 1883;
//const uint16_t http_port = 3000;

 int delayTime = 8000;
int light = 0;
int ledValue = 0;

//MQTT client
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

//Authenticate timer function
void authenticate_rgb(){
  Serial.println("authenticating rgb..");
  client.publish("authenticate", rgb);
}

void authenticate_emrgcy(){
  Serial.println("authenticating emrgcy..");
  client.publish("authenticate", emrgcy);
}

void authenticate_buzzer(){
  Serial.println("authenticating buzzer..");
  client.publish("authenticate", buzzer);
}

void authenticate_door(){
  Serial.println("authenticating door..");
  client.publish("authenticate", door);
}

//mqtt callback function
void callback(char* topic, byte* payload, unsigned int length) {
  String rmsg = "";
//  Serial.println("Message arrived"); 
//  Serial.println("Topic: " + (String)topic);
//  Serial.print("Message: ");
  for (int i = 0; i < length; i++) {
    //Serial.print((char)payload[i]);
    rmsg += (char)payload[i];
  }
  Serial.println(rmsg);

  if((String)topic == "devices/" + rgb_id){
    char light = rmsg.charAt(0);
    if(light=='1'){
       rmsg.remove(0,2);
        long number = strtol( rmsg.c_str(), NULL, 16);
        // Split them up into r, g, b values
        int red_8bit = number >> 16;
        int green_8bit = number >> 8 & 0xFF;
        int blue_8bit = number & 0xFF;
        int red_10bit = red_8bit*1023/255;
        int green_10bit = green_8bit*1023/255;
        int blue_10bit = blue_8bit*1023/255;
        analogWrite(red, red_10bit);
        analogWrite(green, green_10bit);
        analogWrite(blue, blue_10bit);
//        Serial.print("r: "); Serial.println(red_10bit);
//        Serial.print("g: "); Serial.println(green_10bit);
//        Serial.print("b: "); Serial.println(blue_10bit);
    } else if(light=='2'){
      int value = rmsg.charAt(1) - '0';
      digitalWrite(magDoor_pin,value);
    }
    
  } else  if((String)topic =="devices/" + buzzer_id){
    int value = rmsg.charAt(1) - '0';
//    Serial.println(value);
    digitalWrite(bzz_pin,value);
  } else  if((String)topic == "authenticate/rgb"){
    timer.stop(authenticate_timer);
    rgb_id = rmsg;
    String topic = "devices/" + rgb_id;
    client.subscribe(topic.c_str());
    Serial.println(topic.c_str());
    authenticate_timer = timer.every(1000, authenticate_emrgcy);
  } else if((String)topic == "authenticate/emrgcy"){
    timer.stop(authenticate_timer);
    emrgcy_id = rmsg;
    String topic = "devices/" + emrgcy_id;
    client.subscribe(topic.c_str());
    Serial.println(topic.c_str());
    authenticate_timer = timer.every(1000, authenticate_door);
  }else if((String)topic == "authenticate/dss"){
    timer.stop(authenticate_timer);
    door_id = rmsg;
    String topic = "devices/" + door_id;
    client.subscribe(topic.c_str());
    Serial.println(topic.c_str());
    authenticate_timer = timer.every(1000, authenticate_buzzer);
  } else if((String)topic == "authenticate/bzz"){
    timer.stop(authenticate_timer);
    buzzer_id = rmsg;
    String topic = "devices/" + buzzer_id;
    client.subscribe(topic.c_str());
    Serial.println(topic.c_str());
  }
}

// Connect to mqtt broker
void checkMqttConnection() {
  // Loop until we're reconnected
  if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("rgb", "lwt", 0, false, "rgb")) {
      Serial.println("connected");
      //client.publish("connected", "hello");\
      delay(10);
      client.subscribe("authenticate/rgb");
      client.subscribe("authenticate/emrgcy");
      client.subscribe("authenticate/bzz");
      client.subscribe("authenticate/dss");
      authenticate_timer = timer.every(5000, authenticate_rgb);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 3 seconds");
    }
  }
}


void setup() {
    pinMode(red, OUTPUT);
    pinMode(green, OUTPUT);
    pinMode(blue, OUTPUT);
    pinMode(bzz_pin, OUTPUT);
    pinMode(magDoor_pin, OUTPUT);
    pinMode(emrgcy_pin, INPUT);
    pinMode(doorSensor_pin, INPUT);
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
//
//boolean flag = true;
//void readSensors() {
//  int value = digitalRead(emrgcy_pin);
//  if (value&&flag) {
//    String topic  = "devices/sensorModule/" + emrgcy_id;
//    emrgcy = "{\"deviceType\":\"SensorModule\",\"deviceCode\": \"emrgcy\",\"sensors\": [{\"name\": \"Emergency\", \"_type\": \"Emergency\", \"value\": 1}]}";
//    client.publish(topic.c_str(), emrgcy);
////    Serial.println("1");
//    flag = false;
//  }
//  if (!value&&!flag) {
//    String topic  = "devices/sensorModule/" + emrgcy_id;
//    emrgcy = "{\"deviceType\":\"SensorModule\",\"deviceCode\": \"emrgcy\",\"sensors\": [{\"name\": \"Emergency\", \"_type\": \"Emergency\", \"value\": 0}]}";
//    client.publish(topic.c_str(), emrgcy);
////    Serial.println("0");
//    flag = true;
//  }
//}

boolean eValue = false,  dValue = false;
boolean eValue_previous = false, dValue_previous = false;

void readEmrgcySensorValue(){
   eValue = digitalRead(emrgcy_pin);
   if(eValue != eValue_previous){
      eValue_previous = eValue;
      Serial.println(eValue);
      String topic  = "devices/sensorModule/" + emrgcy_id;
      sprintf(emrgcy, "{\"deviceType\":\"SensorModule\",\"deviceCode\": \"emrgcy\",\"sensors\": [{\"name\": \"Emergency\", \"_type\": \"Emergency\", \"value\": %d}]}",  eValue);
      client.publish(topic.c_str(), emrgcy);
   }
}

void readDoorSensorValue(){
   dValue = digitalRead(doorSensor_pin);
   if(dValue != dValue_previous){
      dValue_previous = dValue;
      Serial.println(dValue);
      String topic  = "devices/sensorModule/" + door_id;
      sprintf(door,"{\"deviceType\":\"SensorModule\",\"deviceCode\": \"dss\",\"sensors\": [{\"name\": \"Door\", \"_type\": \"Door\", \"value\": %d}]}",  dValue);
      client.publish(topic.c_str(), door);
   }
}

void loop(){
  client.loop();
  timer.update();
  readEmrgcySensorValue();
  readDoorSensorValue();
}
