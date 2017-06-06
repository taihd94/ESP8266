#include <Arduino.h>

#include <ESP8266WiFi.h>
//#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>
#include <Timer.h>
//#include <ArduinoJson.h>

Timer timer;
int sensor_authenticate_timer;
int buzzer_authenticate_timer;


const char* sensor = "{\"deviceType\":\"SensorModule\",\"deviceCode\": \"sss01\",\"sensors\": [{\"name\": \"Emergency\", \"_type\": \"Emergency\", \"value\": 0}]}";
String sensor_id = "";

const char* buzzer = "{\"deviceType\":\"LightingControl\", \"deviceCode\": \"bzz01\",\"lights\": [{\"portId\": 1,\"name\": \"Buzzer\",\"dimmable\": false,\"typeOfLight\": \"Alarm\"}]}";
String buzzer_id = "";

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

#define pinin 0
#define buzzer_pin 2

//MQTT
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;


//Authenticate
void authenticate_sensor() {
  client.publish("authenticate", sensor);
}

void authenticate_buzzer() {
  client.publish("authenticate", buzzer);
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


  if ((String)topic == "authenticate/sss01") {
    timer.stop(sensor_authenticate_timer);
    sensor_id = rmsg;
    String topic = "devices/" + sensor_id;
    client.subscribe(topic.c_str());
    Serial.println(topic.c_str());
  } else  if ((String)topic == "authenticate/bzz01") {
    timer.stop(buzzer_authenticate_timer);
    buzzer_id = rmsg;
    String topic = "devices/" + buzzer_id;
    client.subscribe(topic.c_str());
    Serial.println(topic.c_str());
  } else {
    int value = rmsg.charAt(1) - '0';
//    Serial.println(value);
    digitalWrite(buzzer_pin,value);
  }


}

// Connect to mqtt broker
void checkMqttConnection() {
  // Loop until we're reconnected
  if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("sss01", "lwt", 0, false, "sss01")) {
      Serial.println("connected");
      //client.publish("connected", "hello");\
      delay(10);
      client.subscribe("authenticate/sss01");
      client.subscribe("authenticate/bzz01");
      sensor_authenticate_timer = timer.every(5000, authenticate_sensor);
      buzzer_authenticate_timer = timer.every(5000, authenticate_buzzer);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 3 seconds");
    }
  }
}
boolean flag = true;
void readSensors() {
  int value = digitalRead(pinin);
  if (value&&flag) {
    String topic  = "devices/sensorModule/" + sensor_id;
    sensor = "{\"deviceType\":\"SensorModule\",\"deviceCode\": \"sss01\",\"sensors\": [{\"name\": \"Emergency\", \"_type\": \"Emergency\", \"value\": 1}]}";
    client.publish(topic.c_str(), sensor);
//    Serial.println("1");
    flag = false;
  }
  if (!value&&!flag) {
    String topic  = "devices/sensorModule/" + sensor_id;
    sensor = "{\"deviceType\":\"SensorModule\",\"deviceCode\": \"sss01\",\"sensors\": [{\"name\": \"Emergency\", \"_type\": \"Emergency\", \"value\": 0}]}";
    client.publish(topic.c_str(), sensor);
//    Serial.println("0");
    flag = true;
  }
}

void setup() {
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
  int cnt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    if (cnt++ > 25) {
      Serial.println();
      cnt = 0;
    }
    delay(100);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(host, mqtt_port);
  client.setCallback(callback);
  timer.every(3000, checkMqttConnection);

    pinMode(pinin, INPUT);
    pinMode(buzzer_pin, OUTPUT);

}

void loop() {
  client.loop();
  timer.update();
  readSensors();
}
