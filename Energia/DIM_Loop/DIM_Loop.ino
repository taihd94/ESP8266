

 #define pinout P1_4
 #define pinin P2_3

 #define light2 P1_3
 #define light3 P1_0
 #define light4 P1_5


String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
unsigned int delayTime = 5000;
int light = 0;
int value2 = 0;
int value3 = 0;
int value4 = 0;

void setup() {
    // initialize serial:
    Serial.begin(9600);
    // reserve 200 bytes for the
    inputString.reserve(200);
    Serial.println("hello");  
    
    pinMode(pinout, OUTPUT);
    pinMode(pinin, INPUT);
    pinMode(light2, OUTPUT);
    pinMode(light3, OUTPUT);
    pinMode(light4, OUTPUT);

//    digitalWrite(pinout,HIGH);
}


void loop() {
 
//  if (stringComplete) {
//    Serial.println(delayTime);
//    // clear the string:
//    inputString = "";
//    stringComplete = false;
//  }


  if(digitalRead(pinin)){
    while(digitalRead(pinin));
    if(!(delayTime==10000)){
      delayMicroseconds(delayTime);
      digitalWrite(pinout,HIGH);
      delayMicroseconds(100);
      digitalWrite(pinout,LOW); 
    }
  }

//  digitalWrite(led, ledValue);

//  Serial.println(digitalRead(pinin));
}

/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
boolean check;
boolean firstChar = true;
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read(); 
    // add it to the inputString:
    if(firstChar){
      light = inChar - '0';
      firstChar = false;
    } else {
      inputString += inChar;
    } 
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      firstChar = true;
      switch(light){
        case 0:
           check = inputString.toInt();
           if((!check)&&(delayTime < 9000)){ 
              delayTime += 1000;
           } else if((check)&&(delayTime > 2000)){
            delayTime -= 1000;
           }
           break;
        case 1:
           delayTime = inputString.toInt();
           break;
        case 2:
//           value2 = inputString.toInt();
           value2 = 1 - value2;
           digitalWrite(light2, value2);
           break;
        case 3:
//           value3 = inputString.toInt();
           value3 = 1 - value3;
           digitalWrite(light3, value3);
           break;
        case 4:
//           value4 = inputString.toInt();
           value4 = 1 - value4;
           digitalWrite(light4, value4);
           break;
        default:
           break;
      }
      //stringComplete = true;
      inputString = "";   
    } 
  }
}


