#include <Servo.h>

//ARUINO INPUTS
int sensorL = A1;
int sensorR = A2;
int wallR = A3;
int wallL = A4;
int wallF = A5;

//CALIBRATED GLOBAL VARIABLES
int lineVoltage = 700;

//NON-CALIBRATED GLOBAL VARIABLES
Servo rightservo;
Servo leftservo;

//wall reads
int read_wallR = 0;
int read_wallL = 0;
int read_wallF = 0;

void setup() {
  Serial.begin(9600);
  servoSetup();
}

int readL[3] = {lineVoltage + 100, lineVoltage + 100, lineVoltage + 100};
int readR[3] = {lineVoltage + 100, lineVoltage + 100, lineVoltage + 100};
double leftAverage;
double rightAverage;

void loop() {
//  while(1) {
//    sample();
//    Serial.println("restart");
//    Serial.print("Right: ");
//    Serial.println(read_wallR);
//    Serial.print("Left: ");
//    Serial.println(read_wallL);
//    Serial.print("Front: ");
//    Serial.println(read_wallF);
//    Serial.print("LineR: ");
//    Serial.println(rightAverage);
//    Serial.print("Line L: ");
//    Serial.println(leftAverage);
//  }
  sample();
  follow();
}

/*Sets up servos*/
void servoSetup()
{
  rightservo.attach(3);
  leftservo.attach(5);
  rightservo.write(90);
  leftservo.write(90);
}

/*All sensor values are read in and assigned to their proper variable in this method*/
void sample()
{
  //LINE SENSORS
  readR[0] = readR[1];
  readR[1] = readR[2];
  readR[2] = analogRead(sensorR);
  readL[0] = readL[1];
  readL[1] = readL[2];
  readL[2] = analogRead(sensorL);
  
  leftAverage = (readL[0] + readL[1] + readL[2])/3;
  rightAverage = (readR[0] + readR[1] + readR[2])/3;
  //END LINE SENSORS

  read_wallR = analogRead(wallR);
  read_wallL = analogRead(wallL);
  read_wallF = analogRead(wallF);
}

/*Uses line sensors to have robots follow the line*/
void follow()
{
  if (rightAverage >= lineVoltage && leftAverage >= lineVoltage) {
    leftservo.write(135);
    rightservo.write(45);
  }
  else if (rightAverage < lineVoltage && leftAverage < lineVoltage) {
    Serial.println("intersection");
    leftservo.write(90);
    rightservo.write(90);
    delay(500);
    
    if (read_wallF >= 100 && read_wallL >= 195 && read_wallR >= 195) {
      Serial.println("uturn");
      turn(1);
      turn(1);
    }
    else if (read_wallF >= 100 && read_wallL < 195 && read_wallR >= 195) {
      Serial.println("left turn");
      turn(0);
    }
    else if (read_wallR < 195) {
      Serial.println("right turn");
      turn(1);
    }
    else {
      leftservo.write(135);
      rightservo.write(45);
    }
  }
  else if (rightAverage < lineVoltage && leftAverage >= lineVoltage) {
    leftservo.write(135);
    rightservo.write(90);
  }
  else if (rightAverage >= lineVoltage && leftAverage < lineVoltage) {
    leftservo.write(90);
    rightservo.write(45);
  }
}

/*Turns the robot according to line sensor readings. A direction of 1 is a right turn.*/
void turn(int direction) {
    int side_passed_once = 0;
    int side_done = 0;
    
    if (direction == 1) { //turn right
      leftservo.write(135);
      rightservo.write(90);
      delay(1000);
//      while(rightAverage < lineVoltage && leftAverage < lineVoltage) {
//        rightAverage = analogRead(sensorR);
//        leftAverage = analogRead(sensorL);
//      }
//      leftservo.write(135);
//      rightservo.write(90);
//      while(!side_done) {
//        sample();
//        if (rightAverage < 700) side_passed_once = 1;
//        if (side_passed_once && rightAverage >= 800) side_done = 1;
//      }
      leftservo.write(135);
      rightservo.write(45);
    }
    else { //turn left
      leftservo.write(90);
      rightservo.write(45);
      delay(500);
//      while(rightAverage < lineVoltage && leftAverage < lineVoltage) {
//        rightAverage = analogRead(sensorR);
//        leftAverage = analogRead(sensorL);
//      }
//      leftservo.write(90);
//      rightservo.write(45);
//      while(!side_done) {
//        sample();
//        if (leftAverage < 700) side_passed_once = 1;
//        if (side_passed_once && leftAverage >= 800) side_done = 1;
//      }
      leftservo.write(135);
      rightservo.write(45);
    }
}

