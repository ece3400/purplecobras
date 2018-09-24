#include <Servo.h>

Servo rightservo;
Servo leftservo;

int sensorM = A1;
int sensorR = A2;
int sensorL = A0;

int turnCount = 0;

void setup() {
  // put your setup code here, to run once:
  rightservo.attach(3);
  leftservo.attach(5);
  rightservo.write(90);
  leftservo.write(90);
  Serial.begin(9600);
}

int readM, readR, readL;

void loop() {
  // put your main code here, to run repeatedly:
  //nothing 5V -> 4.78
  //brown 3V -> 4.5
  //white 0V -> 3.3

  readM = analogRead(sensorM);
  readR = analogRead(sensorR);
  readL = analogRead(sensorL);
  
//  int driveM = map(readM, 0, 1023, 0, 5);
//  int driveR = map(readR, 0, 1023, 0, 5);
//  int driveL = map(readL, 0, 1023, 0, 5);

  float driveM = readM * 5 / 1023.0;
  float driveR = readR * 5 / 1023.0;
  float driveL = readL * 5 / 1023.0;

  Serial.println(readR);
//  Serial.println(driveR);
//  Serial.println(driveL);
  

  if (readR >= 800 && readL >= 800) {
    leftservo.write(135);
    rightservo.write(45);
  }
  else if (readR < 800 && readL>=800) {
    leftservo.write(135);
    rightservo.write(90);
  }
  else if (readR >= 800 && readL < 800) {
    leftservo.write(90);
    rightservo.write(45);
  }
  else if (readR < 800 && readL < 800) {
    Serial.println("Turn");
    turn(true);
  }
}
/*
void turn() {
  if (turnCount % 8 < 4) {  //left turn
    turnCount++;
    leftservo.write(90);
    rightservo.write(45);
    delay(1200);
  }
  else { //right turn
    turnCount++;
    leftservo.write(135);
    rightservo.write(90);
    delay(1200);
  }

}*/

int num_turns = 0;

//right = 1, left = 0
void turn(bool direction) {
    //turn right
    if ( direction ) {
      leftservo.write(90);
      rightservo.write(45);
      while ( num_turns < 2 ) {
        if ( readR < 800 ) {
          num_turns += 1;
        }
      }
      leftservo.write(135);
      rightservo.write(45);
    }
    else {
      leftservo.write(135);
      rightservo.write(90);
      while ( num_turns < 2 ) {
        if ( readL < 800 ) {
          num_turns += 1;
        }
      }
      leftservo.write(135);
      rightservo.write(45);
    }
}


