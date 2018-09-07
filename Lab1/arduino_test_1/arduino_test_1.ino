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
  //nothing 5V
  //brown 3V
  //white 0V

  int reading = analogRead(A0);
  Serial.println(reading);
  
  int drive = map(reading, 0, 1023, 0, 5);
  Serial.println("here");

  
  if (/*4<=*/drive<=5) {
        leftservo.write(90);
        rightservo.write(90);
        delay(50000); 
        Serial.println("stuck");
  }
  else if (/*1.5<=*/drive<4) {
      leftservo.write(0);
      rightservo.write(0);
      delay(100);
      Serial.println("turning");
  }
  else {
    leftservo.write(180);
    rightservo.write(0);
    delay(250);
    Serial.println("straight");
  }
  
  
  /*rightservo.write(180);
  leftservo.write(0);
  delay(1000);
  rightservo.write(0);
  leftservo.write(0);
  delay(675);*/
}
