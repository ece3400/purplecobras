#include <Servo.h>

//ARUINO INPUTS
int sensorL = A4;
int sensorR = A5;
int leftWall = A1;
int frontWall = A3;
int rightWall = A2;

//CALIBRATED GLOBAL VARIABLES
int lineVoltage = 700;
int LRwalls = 200;
int Fwall = 200;

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

enum maze_direction {
  north,
  south,
  east,
  west
};

int rows = 4; //length along y direction
int cols = 5; //length along x direction
char northWall = 0b00001000;
char eastWall = 0b00000100;
char southWall = 0b00000010;
char westWall = 0b00000001;

char front = 0b00001000;
char right = 0b00000100;
char left = 0b00000001;

short maze[5][4] = 
  { {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
  };
  
int moves[60];
int current[2]= {0, 0}; //first is x, second y
int count = 0;
bool backtracking = false;
maze_direction c_direction = north;
char relativeState = 0; // 1 means can go
char absoluteState = 0; // 1 means can go

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
  maze[0][0] = true;
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

void follow() {
    sample();
  if (rightAverage >= lineVoltage && leftAverage >= lineVoltage) {
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
}

void stepPast () {
  leftservo.write(180);
  rightservo.write(0);
  //sample();
  leftAverage = analogRead(sensorL);
  rightAverage = analogRead(sensorR);
  
  while (leftAverage < lineVoltage && rightAverage < lineVoltage) {
    leftAverage = analogRead(sensorL);
    rightAverage = analogRead(sensorR);
  }
  delay(300);
  leftservo.write(90);
  rightservo.write(90);
}

void forward () {
  leftservo.write(135);
  rightservo.write(45);
}

void turnLeft() {
  leftAverage = analogRead(sensorL);
  rightAverage = analogRead(sensorR);
  
  leftservo.write(0);
  rightservo.write(0);
  
  while(leftAverage > lineVoltage && rightAverage > lineVoltage) {
    delay(50);
    leftAverage = analogRead(sensorL);
    rightAverage = analogRead(sensorR);
  }
  while(leftAverage > lineVoltage && rightAverage < lineVoltage) {
    delay(50);
    leftAverage = analogRead(sensorL);
    rightAverage = analogRead(sensorR);
  }
  while(leftAverage > lineVoltage && rightAverage > lineVoltage) {
    delay(50);
    leftAverage = analogRead(sensorL);
    rightAverage = analogRead(sensorR);
  }
  while (leftAverage < lineVoltage && rightAverage > lineVoltage) {
    delay(50);
    leftAverage = analogRead(sensorL);
    rightAverage = analogRead(sensorR);
  }
  leftservo.write(90);
  rightservo.write(90);
}

void turnRight() {
  leftAverage = analogRead(sensorL);
  rightAverage = analogRead(sensorR);
  
  leftservo.write(180);
  rightservo.write(180);
  
  while(leftAverage > lineVoltage && rightAverage > lineVoltage) {
    delay(50);
    leftAverage = analogRead(sensorL);
    rightAverage = analogRead(sensorR);
  }
  while(leftAverage < lineVoltage && rightAverage > lineVoltage) {
    delay(50);
    leftAverage = analogRead(sensorL);
    rightAverage = analogRead(sensorR);
  }
  while(leftAverage > lineVoltage && rightAverage > lineVoltage) {
    delay(50);
    leftAverage = analogRead(sensorL);
    rightAverage = analogRead(sensorR);
  }
  while (leftAverage > lineVoltage && rightAverage < lineVoltage) {
    delay(50);
    leftAverage = analogRead(sensorL);
    rightAverage = analogRead(sensorR);
  }
  leftservo.write(90);
  rightservo.write(90);
}

bool detectRightWall() {
//  digitalWrite(s2, LOW);
//  digitalWrite(s1, LOW);
//  digitalWrite(s0, LOW);

  //read_wallR = analogRead(walls);
  if (analogRead(rightWall) >= LRwalls) {
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
  if (analogRead(frontWall) >= Fwall) {
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
  
  if (analogRead(leftWall) >= LRwalls) {
    return true;
  }
  else {
    return false;
  }
}

char fourBitWrap(char x, int n)
{
  for (int a = 0; a < n; a++)
  {
    char temp = x & 0b00000001;
    temp = temp << 3;
    x = x >> 1;
    x = x & 0b00001111;
    x = x | temp;
  }
  return x;
}

char fourBitWrapLeft(char x, int n)
{
  for (int a = 0; a < n; a++)
  {
    char temp = x & 0b00001000;
    temp = temp >> 3;
    x = x << 1;
    x = x & 0b00001111;
    x = x | temp;
  }
  return x;
}

//i changed the !rWall to be just rWall because you want to or it when there is a wall
void maze_info() {
  if (rWall) {
    relativeState = relativeState | right;
  }
  if (lWall) {
    relativeState = relativeState | left;
  }
  if (fWall) {
    relativeState = relativeState | front;
  }

  absoluteState = relativeState;

  if (c_direction == east){
    absoluteState = fourBitWrap(absoluteState, 1);
  }
  if (c_direction == south){
    absoluteState = fourBitWrap(absoluteState, 2);
  }
  if (c_direction == west){
    absoluteState = fourBitWrap(absoluteState, 3);
  }

  switch(c_direction) {
    case north :
      current[1] = current[1] + 1;
      break;
    case east :
      current[0] = current[0] + 1;
      break;
    case south :
      current[1] = current[1] - 1;
      break;
    case west :
      current[0] = current[0] - 1;
      break;
  }

  if (maze[current[0]][current[1]+1])
  {
    absoluteState = absoluteState & 0b11110111;
  }
  if (maze[current[0]+1][current[1]])
  {
    absoluteState = absoluteState & 0b11111011;
  }
  if (maze[current[0]][current[1]-1])
  {
    absoluteState = absoluteState & 0b11111101;
  }
  if (maze[current[0]-1][current[1]])
  {
    absoluteState = absoluteState & 0b11111110;
  }
  
  maze[current[0]][current[1]] = true;

  relativeState = absoluteState;

  if (c_direction == east){
    relativeState = fourBitWrapLeft(relativeState, 1);
  }
  if (c_direction == south){
    relativeState = fourBitWrapLeft(relativeState, 2);
  }
  if (c_direction == west){
    relativeState = fourBitWrapLeft(relativeState, 3);
  }
}

void backtrack(){
  if (backtracking == false)
  {
    follow();
  }
  backtracking = true;
  if (moves[count] == 1)
  {
    moves[count] = 0;
    count = count - 1;
    turnLeft();
  }
  else if (moves[count] == 2)
  {
    moves[count] = 0;
    count = count - 1;
  }
  else if (moves[count] == 3){
    moves[count] = 0;
    count = count - 1;
    turnRight();
  }
}

void turns() {
//  if ((relativeState & 0b00000100) > 1) {
//    turnRight();
//    moves[count] = 1; //right
//    backtracking = false;
//  }
//  else if ((relativeState & 0b00001000) > 1) {
//    forward();
//    count++;
//    moves[count] = 2; //straight
//    backtracking = false;
//  }
//  else if ((relativeState & 0b00000001) > 1) {
//    turnLeft();
//    moves[count] = 3; //left
//    backtracking = false;
//  }
//  else {
//    if (backtracking) {
//      backtrack();
//    }
//    else {
//      //uturn
//      turnRight();
//      turnRight();
//      backtrack();
//    }
//  }
////  if ( rWall && lWall && fWall ) {
////    Serial.println("walls on all sides");
////    turnLeft();
////    turnLeft();
////  }
////  else if ( rWall && fWall && !lWall ) {
////    Serial.println("no left wall");
////    turnLeft();
////  }
////  else if ( !rWall ) {
////    Serial.println("no right wall");
////    turnRight();
////  }
////  else {
////    
////    forward();
////  }
  // U-turn
    if (fWall && lWall && rWall) {
      turnRight();
      turnRight();
    }
    // Left Turn
    else if (fWall && !lWall && rWall) {
      turnLeft();
    }
    // Right Turn
    else if (!rWall) {
      turnRight();
    }
    // Go forward
    else {
      leftservo.write(135);
      rightservo.write(45);
    }
}

void loop() {
  // put your main code here, to run repeatedly:
  switch (state) {
    case FOLLOW_LINE:
      sample();
      if (rightAverage >= lineVoltage && leftAverage >= lineVoltage) {
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
          Serial.println("Detect Walls");
          leftservo.write(90);
          rightservo.write(90);
          delay(1000); 
          lWall = detectLeftWall();
          rWall = detectRightWall();
          if (rWall) {
            Serial.println("right wall");
          }
          else if ( lWall ) {
            Serial.println("left wall");
          }
          fWall = detectFrontWall();
<<<<<<< HEAD
=======
          if ( fWall ) {
            Serial.println("front Wall");
          }
          action = CHECK;
        case CHECK :
          maze_info();
          for (int i = 0; i < 5; i ++) {
            for (int j = 0; j < 4; j ++) {
              Serial.print(maze[i][j]);
            }
            Serial.println();
          }
>>>>>>> 5bab837f79f26547e6ba02382db2a5bdafa5eaa8
          action = MOVE;
//        case CHECK :
//          maze_info();
//          action = MOVE;
        case MOVE : 
          stepPast();
          turns();
          action = DETECT_WALLS;
        default :
          state = FOLLOW_LINE;
          break;
      }
  }

}
