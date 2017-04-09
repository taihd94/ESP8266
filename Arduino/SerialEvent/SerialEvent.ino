#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

 #define pinout 13
 #define pinin 12

 const char* ssid     = "phuongchi11";
const char* password = "20112009";

#define mqtt_server "192.168.1.102"
#define mqtt_topic_pub "presence"
#define mqtt_topic_sub "presence"
#define mqtt_user "alice"
#define mqtt_pwd "secret"
const uint16_t mqtt_port = 1883;

//MQTT
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete

void setup() {
    pinMode(pinout, OUTPUT);
    pinMode(pinin, INPUT);
  // initialize serial:
  Serial.begin(115200);
  // reserve 200 bytes for the
  inputString:
  inputString.reserve(200);
  Serial.println("hello");

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);

  }

  //set up mqtt
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


//mqtt
void callback(char* topic, byte* payload, unsigned int length) {
  String rmsg = "";
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    rmsg += (char)payload[i];
  }
  Serial.println();
}

//mqtt
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client",mqtt_user,mqtt_pwd)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(mqtt_topic_pub, "ESP_reconnected");
      // ... and resubscribe
      client.subscribe(mqtt_topic_sub);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

int delayTime = 100;

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // print the string when a newline arrives:
  if (stringComplete) {
    Serial.print(inputString);
    delayTime = inputString.toInt(); 
    // clear the string:
    inputString = "";
    stringComplete = false;
  }


  if(digitalRead(pinin)){
    while(digitalRead(pinin));
    delayMicroseconds(delayTime);
    digitalWrite(pinout,HIGH);
    delay(1);
    digitalWrite(pinout,LOW);
  }


  serialEvent();
}

/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}


