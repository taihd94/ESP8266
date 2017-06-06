#include <Arduino.h>

#include <ESP8266WiFi.h>
//#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
#include <Timer.h>
//#include <ArduinoJson.h>

Timer timer;
int authenticate_timer;


const char* device = "{\"deviceType\":\"LightingControl\",\"deviceCode\": \"rgb\",\"numberOfPorts\": 1, \"lights\": [{\"portId\": 1, \"dimmable\": false, \"typeOfLight\": \"RGB\"}]}";
String device_Id = "";

const char* ssid     = "phuongchi11";
const char* password = "20112009";

//khai bao mqtt
#define host "192.168.1.103"
#define mqtt_topic_pub "presence"
#define mqtt_topic_sub "presence"
#define mqtt_topic_lwt "lwt"
#define mqtt_user "alice"
#define mqtt_pwd "secret"
const uint16_t mqtt_port = 1883;
const uint16_t http_port = 3000;

 #define red 16
 #define green 13
 #define blue 15


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
  Serial.println("authenticating..");
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


  if((String)topic == "authenticate/rgb"){
    timer.stop(authenticate_timer);
    device_Id = rmsg;
    String topic = "devices/" + device_Id;
    client.subscribe(topic.c_str());
    Serial.println(topic.c_str());
  } else {
    char light = rmsg.charAt(0);
    switch(light){
     case '1':
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
        Serial.print("r: "); Serial.println(red_10bit);
        Serial.print("g: "); Serial.println(green_10bit);
        Serial.print("b: "); Serial.println(blue_10bit);
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
    if (client.connect("rgb", "lwt", 0, false, "rgb")) {
      Serial.println("connected");
      //client.publish("connected", "hello");\
      delay(10);
      client.subscribe("authenticate/rgb");
      authenticate_timer = timer.every(5000, authenticate);
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
//  readLightsStatus();
}
