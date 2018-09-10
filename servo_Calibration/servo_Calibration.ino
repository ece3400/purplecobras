#include <Servo.h>

Servo rightservo;
Servo leftservo;

void setup() {
  // put your setup code here, to run once:
  rightservo.attach(3);
  leftservo.attach(5);
  rightservo.write(90);
  leftservo.write(90);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  leftservo.write(90);
  rightservo.write(90);
  delay(50000); 

  
}
