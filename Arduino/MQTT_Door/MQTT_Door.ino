#define IN1     2
#define IN2     0
#define button1 5
#define button2 4
#define button3 13
 
 
void setup()
{
  Serial.begin(9600);
  pinMode( IN1,      OUTPUT);
  pinMode( IN2,      OUTPUT);
  pinMode( button1,  INPUT); 
  pinMode( button2,  INPUT);
  pinMode( button3,  INPUT);
  Serial.println("hello");
}


void tien()
{
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  }


void dung()
{
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  }


void lui()
{
   digitalWrite(IN1, LOW);
   digitalWrite(IN2, HIGH);
  }


void loop() 
{
//  Serial.println(digitalRead(button1));
  if(digitalRead(button3))
  { 
    tien();
    Serial.println("ok1");
    while (!digitalRead(button1)){delay(10);};
    Serial.println("ok2");
    dung();
    delay (3000);
    lui();
    Serial.println("ok3");
    while (!digitalRead(button2)){delay(10);};
    Serial.println("ok4");
    dung();
  }   
  else
  {
    dung();
  }
}
