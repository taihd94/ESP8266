#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Timer.h>
#include <SoftwareSerial.h>
#include <FPM.h>

//define timer
Timer timer;
int authenticate_timer;

//define access point
const char* ssid     = "BKHome";
const char* password = "bkhomebkhome";

//define MQTT client
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;


//define mqtt params
#define host "192.168.10.100"
#define mqtt_topic_pub "presence"
#define mqtt_topic_sub "presence"
#define mqtt_topic_lwt "lwt"
#define mqtt_user "alice"
#define mqtt_pwd "secret"
const uint16_t mqtt_port = 1883;

String user_enrol_id = "";

//define fingerprint params
#define TEMPLATES_PER_PAGE  256
SoftwareSerial mySerial(12, 14);
FPM finger;
int getFingerprintEnroll(int id);
int command_Id = 1;

//unsigned int hash(unsigned int x)
//{
//    x = ((x >> 16) ^ x) * 0x45d9f3b;
//    x = ((x >> 16) ^ x) * 0x45d9f3b;
//    x = (x >> 16) ^ x;
//    return x;
//}

void setup() {
   Serial.begin(9600);
   delay(10);

  mySerial.begin(57600);
  delay(10);

  //Serial.setDebugOutput(true);

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

  if (finger.begin(&mySerial)) {
    Serial.println("Found fingerprint sensor!");
    Serial.print("Capacity: "); Serial.println(finger.capacity);
    Serial.print("Packet length: "); Serial.println(finger.packetLen);
  } else {
    Serial.println("Did not find fingerprint sensor :(");
  }
  timer.every(5000, checkMqttConnection);
}

//********************MQTT********************//
//mqtt callback function
void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
//  Serial.print("Message arrived: [");
//  Serial.print(topic);
//  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    //Serial.print((char)payload[i]);
    msg += (char)payload[i];
  }
  Serial.println(topic);
  Serial.println(msg);

  if((String)topic=="fgss1/authenticate"){
    Serial.println("Waiting for a finger...");
    command_Id = 1;
  }
  if((String)topic == "fgss1/enrol"){
     Serial.println("Waiting for valid finger to enroll");
     user_enrol_id = msg;
    command_Id = 2;
  }
  if((String)topic=="fgss1/deleteFingerprints"){
//     int id = msg.toInt();
     char buf[10];
     msg.toCharArray(buf, 10);
     char *token;
     token = strtok(buf, ",");
     while(token!=NULL){
        int id = atoi(token);
        deleteFingerprint(id);
        token  = strtok(NULL, ",");
     }
  }
}

// Connect to mqtt broker
void checkMqttConnection() {
  // Loop until we're reconnected
  if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("fgss1", "lwt", 0, false, "fgss1")) {
      Serial.println("connected");
//      client.publish("access-control", "hello from esp8266");
      client.subscribe("fgss1/authenticate");
      client.subscribe("fgss1/enrol");
      client.subscribe("fgss1/deleteFingerprints");
//        timer.every(100,  fingerprint);

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
    }
  }
}

//********************FINGERPRINT***********************//

//--------------------------fingerprint select function---------------//
void fingerprint(){
  switch(command_Id){
    case 1:
        getFingerprintID();
        break;
    case 2:
        enrolFingerprint();
        break;
    default:
        Serial.println("id is invalid");
  }
}

// -------------------------get Fingerprint ID----------------------//

int getFingerprintID() {
  int p = -1;
  p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  Serial.println("Remove finger...");
  // client.publish("access-control", "Remove your finger...");
  while (p != FINGERPRINT_NOFINGER){
    p = finger.getImage();
  }
  Serial.println();
  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
//  unsigned _id = hash(finger.fingerID);
//  Serial.println(_id);
  char buf [10];
  sprintf (buf, "%u", finger.fingerID);
  Serial.println(buf);
  client.publish("access-control/fingerprint/authenticate/found-id", buf);
}
// --------------------------------------------------------------------------//

//-------------------------Enrol fingerprint----------------------------//
int enrolFingerprint(){
//  Serial.println("Send any character to enroll a finger...");
//  while (Serial.available() == 0);
//  Serial.println("Searching for a free slot to store the template...");
  int16_t id;
  if (get_free_id(&id)){
    int isOK = !getFingerprintEnroll(id);
    if(isOK){
      Serial.println("Waiting for a finger...");
      command_Id = 1;
    }
  }  else  Serial.println("No free slot in flash library!");
  while (Serial.read() != -1);  // clear buffer just in case
}

bool get_free_id(int16_t * id){
  int p = -1;
  for (int page = 0; page < (finger.capacity / TEMPLATES_PER_PAGE) + 1; page++){
    p = finger.getFreeIndex(page, id);
    switch (p){
      case FINGERPRINT_OK:
        if (*id != FINGERPRINT_NOFREEINDEX){
//          Serial.print("Free slot at ID ");
//          Serial.println(*id);
          return true;
        }
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error!");
        return false;
      default:
        Serial.println("Unknown error!");
        return false;
    }
  }
}

int getFingerprintEnroll(int id) {
  int p = -1;
  p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      //client.publish("access-control/fingerprint/enrol/message", "Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
     return p;
  }
  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      //client.publish("access-control/fingerprint/enrol/message", "Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      client.publish("access-control/fingerprint/enrol/error", "Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
       client.publish("access-control/fingerprint/enrol/error", "Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
       client.publish("access-control/fingerprint/enrol/error", "Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
       client.publish("access-control/fingerprint/enrol/error", "Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
       client.publish("access-control/fingerprint/enrol/error", "Unknown error");
      return p;
  }

  Serial.println("Remove finger...");
  client.publish("access-control/fingerprint/enrol/message", "Remove your finger...");

  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.println();
  // OK converted!

  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.print("Found ID #"); Serial.print(finger.fingerID);
    Serial.print(" with confidence of "); Serial.println(finger.confidence);
    Serial.println("Finger already enrolled");
    Serial.println("Please place another finger...");
    client.publish("access-control/fingerprint/enrol/message", "Finger already enrolled. Please place another finger...");
    return 1;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    client.publish("access-control/fingerprint/enrol/error", "Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
//    Serial.println("Did not find a match");
  } else {
    Serial.println("Unknown error");
    client.publish("access-control/fingerprint/enrol/error", "Unknown error");
    return p;
  }
  //Did not find a match

  p = -1;
  Serial.println("Place same finger again");
  client.publish("access-control/fingerprint/enrol/message", "Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      //client.publish("access-control/fingerprint/enrol/message", "Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      client.publish("access-control/fingerprint/enrol/error", "Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      client.publish("access-control/fingerprint/enrol/error", "Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      client.publish("access-control/fingerprint/enrol/error", "Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      //client.publish("access-control/fingerprint/enrol/message", "Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      client.publish("access-control/fingerprint/enrol/error", "Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      client.publish("access-control/fingerprint/enrol/error", "Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      client.publish("access-control/fingerprint/enrol/error", "Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      client.publish("access-control/fingerprint/enrol/error", "Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      client.publish("access-control/fingerprint/enrol/error", "Unknown error");
      return p;
  }


  // OK converted!
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
//   client.publish("access-control/fingerprint/enrol/message", "Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    client.publish("access-control/fingerprint/enrol/error", "Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    client.publish("access-control/fingerprint/enrol/error", "Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    client.publish("access-control/fingerprint/enrol/error", "Unknown error");
    return p;
  }

  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
     client.publish("access-control/fingerprint/enrol/message", "Complete!");
//     unsigned int _id = hash(id);
     String message = "{\"userId\": \"" + user_enrol_id + "\", \"fingerprintId\": " + id + "}";
     Serial.println(message);
     client.publish("access-control/fingerprint/enrol/complete", message.c_str());
    return p;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
     client.publish("access-control/fingerprint/enrol/error", "Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
     client.publish("access-control/fingerprint/enrol/error", "Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
     client.publish("access-control/fingerprint/enrol/error", "Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
     client.publish("access-control/fingerprint/enrol/error", "Unknown error");
    return p;
  }
}

//--------------------------------------------------------------//

//---------------------Delete fingerprint------------------//
int deleteFingerprint(int id) {
  int p = -1;

  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {
    Serial.println("Deleted!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not delete in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
    return p;
  }
}

//--------------------------------------------------------------------//


//forever loop
void loop(){
  client.loop();
  timer.update();
  fingerprint();
}
