#include <Servo.h>

Servo rightservo;
Servo leftservo;

int sensorM = A0;
int sensorR = A1;
int sensorL = A2;

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
  //nothing 5V -> 4.78
  //brown 3V -> 4.5
  //white 0V -> 3.3

  int readM = analogRead(sensorM);
  int readR = analogRead(sensorR);
  int readL = analogRead(sensorL);

  Serial.println(readR + " " + readM + " " + readL);
  
  int driveM = map(reading, 0, 1023, 0, 5);
  int driveR = map(reading, 0, 1023, 0, 5);
  int driveL = map(reading, 0, 1023, 0, 5);
  
  Serial.println("here");

  if(driveM <= 3.3 && (driveR > 3.3 && driveL > 3.3)) {
    leftservo.write(180);
    rightservo.write(0);
    delay(250);
    Serial.println("straight");
  }
  if(driveM >= 4.78 || driveR >= 4.78 || driveL >= 4.78){
    leftservo.write(90);
    rightservo.write(90);
    delay(50000); 
    Serial.println("stuck");
  }
}

void turnL() {
  if (driveM <=3.3 && driveR <= 3.3 && driveL <=3.3) {
     leftservo.write(0);
    rightservo.write(0);
    delay(100);
    Serial.println("turning");
  }
}

void turnR() {

}

