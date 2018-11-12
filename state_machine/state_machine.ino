#include <Servo.h>

//ARUINO INPUTS
int sensorL = A1;
int sensorR = A2;
int leftWall = A3;
int frontWall = A4;
int rightWall = A5;

//CALIBRATED GLOBAL VARIABLES
int lineVoltage = 600;
int LRwalls = 155;
int Fwall = 100;

//NON-CALIBRATED GLOBAL VARIABLES
Servo rightservo;
Servo leftservo;

bool lWall;
bool rWall;
bool fWall;

//states for the outside loop
enum states {
  DETECT_SOUND,
  FOLLOW_LINE,
  INTERSECTION,
};

//states to do within the intersection
enum actions {
  DETECT_WALLS,
  MOVE,
  DETECT_ROBOT,
  SEND_RADIO,
  CHECK
};

states state = FOLLOW_LINE;
actions action = DETECT_WALLS;

/*Sets up servos*/
void servoSetup()
{
  rightservo.attach(3);
  leftservo.attach(5);
  rightservo.write(90);
  leftservo.write(90);
}

void setup() {
  Serial.begin(9600);
  servoSetup();
  state = FOLLOW_LINE;
}

int readL[3] = {lineVoltage + 100, lineVoltage + 100, lineVoltage + 100};
int readR[3] = {lineVoltage + 100, lineVoltage + 100, lineVoltage + 100};
double leftAverage;
double rightAverage;

void sample()
{
  readR[0] = readR[1];
  readR[1] = readR[2];
  readR[2] = analogRead(sensorR);
  readL[0] = readL[1];
  readL[1] = readL[2];
  readL[2] = analogRead(sensorL);
  
  leftAverage = (readL[0] + readL[1] + readL[2])/3;
  rightAverage = (readR[0] + readR[1] + readR[2])/3;
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
    Serial.println("one on black");
    sample();
  }
  while (leftAverage < lineVoltage || rightAverage < lineVoltage) {
    Serial.println("one on white");
    sample();
  }

  leftservo.write(90);
  rightservo.write(90);
}

void turnRight() {
  leftservo.write(135);
  rightservo.write(90);
  
  sample();
  
  while(leftAverage > lineVoltage || rightAverage > lineVoltage) {
    Serial.println("one on black");
    sample();
  }
  while (leftAverage < lineVoltage || rightAverage < lineVoltage) {
    Serial.println("one on white");
    sample();
  }

  leftservo.write(90);
  rightservo.write(90);
}

bool detectRightWall() {
//  digitalWrite(s2, LOW);
//  digitalWrite(s1, LOW);
//  digitalWrite(s0, LOW);

  //read_wallR = analogRead(walls);

  if (analogRead(A5) >= LRwalls) {
    return true;
  }
  else {
    return false;
  }
}

bool detectFrontWall() {
//  digitalWrite(s2, LOW);
//  digitalWrite(s1, LOW);
//  digitalWrite(s0, HIGH);

  //int read_wallF = analogRead(walls);

  if (analogRead(A4) >= Fwall) {
    return true;
  }
  else {
    return false;
  }
}

bool detectLeftWall() {
//  digitalWrite(s2, LOW);
//  digitalWrite(s1, HIGH);
//  digitalWrite(s0, LOW);

  //read_wallL = analogRead(walls);

  
  if (analogRead(A3) >= LRwalls) {
    return true;
  }
  else {
    return false;
  }
}

void turns() {
  if ( rWall && lWall && fWall ) {
    turnLeft();
    turnLeft();
  }
  else if ( rWall && fWall && !lWall ) {
    turnLeft();
  }
  else if ( !rWall ) {
    turnRight();
  }
  else {
    forward();
  }
}
void loop() {
  // put your main code here, to run repeatedly:
  switch (state) {
    case FOLLOW_LINE:
      sample();
      if (rightAverage >= lineVoltage && leftAverage >= lineVoltage) {
        leftservo.write(135);
        forward();
        state = FOLLOW_LINE;
      }
      else if (rightAverage < lineVoltage && leftAverage < lineVoltage) {
        state = INTERSECTION;
      }
      // Continue turning right
      else if (rightAverage < lineVoltage && leftAverage >= lineVoltage) {
        leftservo.write(135);
        rightservo.write(90);
        state = FOLLOW_LINE;
      }
      // Continue turning left
      else if (rightAverage >= lineVoltage && leftAverage < lineVoltage) {
        leftservo.write(90);
        rightservo.write(45);
        state = FOLLOW_LINE;
      }
      break;
    case INTERSECTION:
      switch (action) {
        case DETECT_WALLS : 
          lWall = detectLeftWall;
          rWall = detectRightWall;
          fWall = detectFrontWall;
        case MOVE : 
          turns();
        default :
          state = FOLLOW_LINE;
          break;
      }
  }

}
