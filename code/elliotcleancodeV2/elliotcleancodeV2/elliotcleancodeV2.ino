#include <Servo.h>

//ARUINO INPUTS
int sensorL = A4;
int sensorR = A5;

//CALIBRATED GLOBAL VARIABLES
int lineVoltage = 800;

//NON-CALIBRATED GLOBAL VARIABLES
Servo rightservo;
Servo leftservo;

void setup() {
  servoSetup();
  Serial.begin(9600);
}

int readL[3] = {lineVoltage + 100, lineVoltage + 100, lineVoltage + 100};
int readR[3] = {lineVoltage + 100, lineVoltage + 100, lineVoltage + 100};
double leftAverage;
double rightAverage;

void loop() {
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
}

/*Uses line sensors to have robots follow the line*/
void follow()
{
  if (rightAverage >= lineVoltage && leftAverage >= lineVoltage) {
    forward();
  }
  else if (rightAverage < lineVoltage && leftAverage >= lineVoltage) {
    leftservo.write(135);
    rightservo.write(90);
  }
  else if (rightAverage >= lineVoltage && leftAverage < lineVoltage) {
    leftservo.write(90);
    rightservo.write(45);
  }
  else if (rightAverage < lineVoltage && leftAverage < lineVoltage) {
    turnRight();
  }
}

void forward() {
  leftservo.write(135);
  rightservo.write(45);
}

void turnLeft() {
  leftservo.write(90);
  rightservo.write(45);
  
  sample();
  
  while(leftAverage > lineVoltage || rightAverage > lineVoltage) {
    sample();
  }
  while (leftAverage < lineVoltage || rightAverage < lineVoltage) {
    sample();
  }

  leftservo.write(90);
  rightservo.write(90);
}

void turnRight() {
  leftservo.write(180);
  rightservo.write(180);
  
  //sample();
  leftAverage = analogRead(A4);
  rightAverage = analogRead(A5);
  
  while(leftAverage > lineVoltage || rightAverage > lineVoltage) {
    //sample();
    leftAverage = analogRead(A4);
    rightAverage = analogRead(A5);
  }
  while (leftAverage < lineVoltage || rightAverage < lineVoltage) {
    //sample();
    leftAverage = analogRead(A4);
    rightAverage = analogRead(A5);
  }

  leftservo.write(90);
  rightservo.write(90);
}
/*Turns the robot according to line sensor readings. A direction of 1 is a right turn.*/
void turn(int direction) {
    int side_passed_once = 0;
    int side_done = 0;
    
    if (direction) { //turn right
      leftservo.write(135);
      rightservo.write(45);
      while(rightAverage < lineVoltage && leftAverage < lineVoltage) {
        rightAverage = analogRead(sensorR);
        leftAverage = analogRead(sensorL);
      }
      leftservo.write(135);
      rightservo.write(90);
      while(!side_done) {
        sample();
        if (rightAverage < 700) side_passed_once = 1;
        if (side_passed_once && rightAverage >= 800) side_done = 1;
      }
      leftservo.write(135);
      rightservo.write(45);
    }
    else { //turn left
      leftservo.write(135);
      rightservo.write(45);
      while(rightAverage < lineVoltage && leftAverage < lineVoltage) {
        rightAverage = analogRead(sensorR);
        leftAverage = analogRead(sensorL);
      }
      leftservo.write(90);
      rightservo.write(45);
      while(!side_done) {
        sample();
        if (leftAverage < 700) side_passed_once = 1;
        if (side_passed_once && leftAverage >= 800) side_done = 1;
      }
      leftservo.write(135);
      rightservo.write(45);
    }
}
