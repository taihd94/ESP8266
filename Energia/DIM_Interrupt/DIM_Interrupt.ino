

 #define pinout P1_4
 #define pinin P1_5


String inputString = "";         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
volatile int delayTime = 1000;

void setup() {
    // initialize serial:
    Serial.begin(9600);
    // reserve 200 bytes for the
    inputString:
    inputString.reserve(200);
    Serial.println("hello");  
    
    pinMode(pinout, OUTPUT);
    //pinMode(pinin, INPUT);
    pinMode(pinin, INPUT_PULLUP);
    attachInterrupt(pinin, handleInterrupt, FALLING); // Interrupt is fired whenever button is pressed
  

}

void handleInterrupt(){
    delayMicroseconds(delayTime);
    digitalWrite(pinout,HIGH);
    delayMicroseconds(500);
    digitalWrite(pinout,LOW);
}


void loop() {
 
//  if (stringComplete) {
//    Serial.print(inputString); 
//    // clear the string:
//    inputString = "";
//    stringComplete = false;
//  }

//
//  if(digitalRead(pinin)){
//    while(digitalRead(pinin));
//    delayMicroseconds(delayTime);
//    digitalWrite(pinout,HIGH);
//    delay(1);
//    digitalWrite(pinout,LOW);
//  }


}

/*
  SerialEvent occurs whenever a new data comes in the
 hardware serial RX.  This routine is run between each
 time loop() runs, so using delay inside loop can delay
 response.  Multiple bytes of data may be available.
 */
void serialEvent() {
  detachInterrupt(pinin);
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read(); 
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:
    if (inChar == '\n') {
      delayTime = inputString.toInt();
      stringComplete = true;    
      Serial.print(inputString); 
    // clear the string:
      inputString = "";
      stringComplete = false;  
    } 
  }
   attachInterrupt(pinin, handleInterrupt, FALLING);
}


