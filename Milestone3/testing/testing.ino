#include <Servo.h>

//ARUINO INPUTS
int sensorL = A1;
int sensorR = A2;

#define LOG_OUT 1 // use the log output function
#define FFT_N 128 // set to 128 point fft
#include <FFT.h>

//wall selector
int s2 = 12;
int s1 = 11;
int s0 = 10;
//wall sensor
int walls = A3;

//wall mux
//000 Left
//001 Right
//010 Front


//CALIBRATED GLOBAL VARIABLES
int lineVoltage = 400;
int LRwalls = 155;
int Fwall = 100;
int pause = 1200;
int mic_threshold = 40;

//NON-CALIBRATED GLOBAL VARIABLES
Servo rightservo;
Servo leftservo;

////wall reads
//int read_wallR = 0;
//int read_wallL = 0;
//int read_wallF = 0;

enum maze_direction {
  North,
  East,
  South,
  West
};

maze_direction m_direction = North;

enum action {
  UTURN,
  LEFT,
  RIGHT,
  FORWARD
};

action next = FORWARD;

bool maze[4][5];
int path[20][2];
int current[]= {3, 0}; //first is rows, second columns
maze_direction c_direction = North;
int pointer = 1;

void setup() {
  //setting up maze
  maze[3][0] = true;
  
  //wall selects
  pinMode(s2, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s0, OUTPUT);
  
  Serial.begin(9600);
  //printf_begin();
  
  //LED for robot detection
  pinMode(7, OUTPUT);
  servoSetup();

  int mic = 0;
  while (mic == 0) {
    mic = detectMicrophone();
    Serial.println("Waiting for mic");
  }

}

int readL[3] = {lineVoltage + 100, lineVoltage + 100, lineVoltage + 100};
int readR[3] = {lineVoltage + 100, lineVoltage + 100, lineVoltage + 100};
double leftAverage;
double rightAverage;

void loop() {
  Serial.println("------------");
  for(int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 5; j++) {
      Serial.print(maze[i][j]);
    }
    Serial.println();
  }
  Serial.println("------------");
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
  // Go forward
  if (rightAverage >= lineVoltage && leftAverage >= lineVoltage) {
    leftservo.write(135);
    rightservo.write(45);
  }
  // intersection
  else if (rightAverage < lineVoltage && leftAverage < lineVoltage) {
    intersection();
  }
  // Continue turning right
  else if (rightAverage < lineVoltage && leftAverage >= lineVoltage) {
    leftservo.write(135);
    rightservo.write(90);
  }
  // Continue turning left
  else if (rightAverage >= lineVoltage && leftAverage < lineVoltage) {
    leftservo.write(90);
    rightservo.write(45);
  }
}

void intersection(){
// not sure if this stopping is necessary or not
  leftservo.write(90);
  rightservo.write(90);
  delay(500);
  bool rWall = detectRightWall();
  bool lWall = detectLeftWall();
  bool fWall = detectFrontWall();
  
  switch(c_direction) { //update current location
    case North :
      current[0] = current[0] + 1;
    case East :
      current[1] = current[1] + 1;
    case South :
      current[0] = current[0] - 1;
    case West :
      current[1] = current[1] - 1;
  }

  maze[current[0]][current[1]] = true;
  Serial.println(c_direction);
  
  path[pointer][0] = current[0];
  path[pointer][1] = current[1];
  
  if (rWall && lWall && fWall) {
    Serial.println("HELP");
    backTrack();
    
  }
  else if (rWall && fWall && !lWall) {
    if (c_direction == North) {
      if (!maze[current[0]][current[1]-1]) {
        moveWest();
        pointer ++;
      }
      else {
        backTrack();
      }
    }
    else if (c_direction == East) {
      if (!maze[current[0]+1][current[1]]) {
        moveNorth();
        pointer ++;
      }
      else {
        backTrack();
      }
    }
    else if (c_direction == South) {
      if (!maze[current[0]][current[1]+1]) {
        moveEast();
        pointer++;
      }
      else {
        backTrack();
      }
    }
    else {
      if (!maze[current[0]-1][current[1]]) {
        moveSouth();
        pointer++;
      }
      else {
        backTrack();
      }
    }
  }
  else if (!rWall) {
    Serial.println("help");
    if (c_direction == North) {
      if (!maze[current[0]][current[1]+1]) {
        Serial.println("MOVE EAST");
        moveEast();
        pointer++;
      }
      else {
        if (!fWall && !lWall) {
          if (!maze[current[0]+1][current[1]]) {
            Serial.println("MOVE NORTH");
            moveNorth();
            pointer++;
          }
          else if (!maze[current[0]][current[1]-1]) {
            Serial.println("MOVE WEST");
            moveWest();
            pointer++;
          }
          else {
            backTrack();
          }
        }
        else if (!fWall) {
          if (!maze[current[0]+1][current[1]]) {
            Serial.println("MOVE NORTH");
            moveNorth();
            pointer++;
          }
          else {
            backTrack();
          }
        }
        else if (!lWall) {
          if (!maze[current[0]][current[1]-1]) {
            Serial.println("MOVE WEST");
            moveWest();
            pointer++;
          }
          else {
            backTrack();
          }
        }
        else {
          backTrack();
        }
      }
    }
    else if (c_direction == East) {
      if (!maze[current[0]-1][current[1]]) {
        Serial.println("MOVE SOUTH");
        moveSouth();
        pointer++;
      }
      else {
        if (!fWall && !lWall) {
          if (!maze[current[0]][current[1]+1]) {
            Serial.println("MOVE EAST");
            moveEast();
            pointer++;
          }
          else if (!maze[current[0]+1][current[1]]) {
            Serial.println("MOVE NORTH");
            moveNorth();
            pointer++;
          }
          else {
            backTrack();
          }
        }
        else if (!fWall) {
          if (!maze[current[0]][current[1]+1]) {
            Serial.println("MOVE EAST");
            moveEast();
            pointer++;
          }
          else {
            backTrack();
          }
        }
        else if (!lWall) {
          if (!maze[current[0]+1][current[1]]) {
            Serial.println("MOVE NORTH");
            moveNorth();
            pointer++;
          }
          else {
            backTrack();
          }
        }
        else {
          backTrack();
        }
      }
    }
    else if (c_direction == South) {
      if (!maze[current[0]][current[1]-1]) {
        Serial.println("MOVE SOUTH");
        moveWest();
        pointer++;
      }
      else {
        if (!fWall && !lWall) {
          if (!maze[current[0]-1][current[1]]) {
            Serial.println("MOVE SOUTH");
            moveSouth();
            pointer++;
          }
          else if (!maze[current[0]][current[1]+1]) {
            Serial.println("MOVE EAST");
            moveEast();
            pointer++;
          }
          else {
            backTrack();
          }
        }
        else if (!fWall) {
          if (!maze[current[0]-1][current[1]]) {
            Serial.println("MOVE SOUTH");
            moveSouth();
            pointer++;
          }
          else {
            backTrack();
          }
        }
        else if (!lWall) {
          if (!maze[current[0]][current[1]+1]) {
            Serial.println("MOVE EAST");  
            moveEast();
            pointer++;
          }
          else {
            backTrack();
          }
        }
        else {
          backTrack();
        }
      }
    }
    else {
      if (!maze[current[0]+1][current[1]]) {
        moveNorth();
        pointer++;
      }
      else {
        if (!fWall && !lWall) {
          if (!maze[current[0]][current[1]-1]) {
            moveWest();
            pointer++;
          }
          else if (!maze[current[0]-1][current[1]]) {
            moveSouth();
            pointer++;
          }
          else {
            backTrack();
          }
        }
        else if (!fWall) {
          if (!maze[current[0]][current[1]-1]) {
            moveWest();
            pointer++;
          }
          else {
            backTrack();
          }
        }
        else if (!lWall) {
          if (!maze[current[0]-1][current[1]]) {
            moveSouth();
            pointer++;
          }
          else {
            backTrack();
          }
        }
        else {
          backTrack();
        }
      }
    }
  }
  else {
    if (c_direction == North) {
      if (!maze[current[0]+1][current[1]]) {
        moveNorth();
        pointer++;
      }
      else if (!lWall) {
        if (!maze[current[0]][current[1]-1]) {
          moveWest();
          pointer++;
        }
        else {
          backTrack();
        }
      }
      else {
        backTrack();
      }
    }
    else if (c_direction == East) {
      if (!maze[current[0]+1][current[1]]) {
        moveEast();
        pointer++;
      }
      else if (!lWall) {
        if (!maze[current[0]+1][current[1]]) {
          moveNorth();
          pointer++;
        }
        else {
          backTrack();
        }
      }
      else {
        backTrack();
      }
    }
    else if (c_direction == South) {
      if (!maze[current[0]-1][current[1]]) {
        moveSouth();
        pointer++;
      }
      else if (!lWall) {
        if (!maze[current[0]][current[1]+1]) {
          moveEast();
          pointer++;
        }
        else {
          backTrack();
        }
      }
      else {
        backTrack();
      }
    }
    else {
      if (!maze[current[0]][current[1]-1]) {
        moveWest();
        pointer++;
      }
      else if (!lWall) {
        if (!maze[current[0]-1][current[1]]) {
          moveSouth();
          pointer++;
        }
        else {
          backTrack();
        }
      }
      else {
        backTrack();
      }
    }
  }
}

/*Turns the robot according to line sensor readings. A direction of 1 is a right turn.*/
void turn(int direction) {
    int side_passed_once = 0;
    int side_done = 0;
    
    if (direction == 1) { //turn right
      leftservo.write(135);
      rightservo.write(90);
    }
    else if (direction == 0) { //turn left
      leftservo.write(90);
      rightservo.write(45);
    }
    else {
      leftservo.write(135);
      rightservo.write(135);
      delay(200);
    }

    delay(pause);
//    leftservo.write(135);
//    rightservo.write(45);
}

bool detectRightWall() {
  digitalWrite(s2, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s0, LOW);

  //read_wallR = analogRead(walls);

  if (analogRead(walls) >= LRwalls) {
    return true;
  }
  else {
    return false;
  }
}

bool detectFrontWall() {
  digitalWrite(s2, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s0, HIGH);

  //int read_wallF = analogRead(walls);

  Serial.println(analogRead(walls));
  if (analogRead(walls) >= Fwall) {
    return true;
  }
  else {
    return false;
  }
}

bool detectLeftWall() {
  digitalWrite(s2, LOW);
  digitalWrite(s1, HIGH);
  digitalWrite(s0, LOW);

  //read_wallL = analogRead(walls);

  
  if (analogRead(walls) >= LRwalls) {
    return true;
  }
  else {
    return false;
  }
}

void moveNorth() {
  if (c_direction == North) {
    //move forward
    leftservo.write(135);
    rightservo.write(45);
    
  }
  if (c_direction == East) {
    //turn left
    turn(0);
  }
  if (c_direction == South) {
    //uturn
    turn(2);
    
  }
  if (c_direction == West) {
    //turn right
    turn(1);
  }
  c_direction = North;
}

void moveSouth() {
  if (c_direction == North) {
    //uturn
    turn(2);
  }
  if (c_direction == East) {
    //turn right
    turn(1);
  }
  if (c_direction == South) {
    leftservo.write(135);
    rightservo.write(45);
  }
  if (c_direction == West) {
    //turn left
    turn(0);
  }
  c_direction = South;
}

void moveEast() {
  if (c_direction == North) {
    //turn right
    turn(1);
  }
  if (c_direction == East) {
    leftservo.write(135);
    rightservo.write(45);
  }
  if (c_direction == South) {
    //turn left
    turn(0);
  }
  if (c_direction == West) {
    //uturn
    turn(2);
  }
  c_direction = East;
}

void moveWest() {
  if (c_direction == North) {
    //turn left
    turn(0);
  }
  if (c_direction == East) {
    //uturn
    turn(2);
  }
  if (c_direction == South) {
    //turn right
    turn(1);
  }
  if (c_direction == West) {
    leftservo.write(135);
    rightservo.write(45);
  }
  c_direction = West;
}
//void change_direction(int how_many_turn) {
//  switch( how_many_turn ) {
//    // turn right
//    case 0:
//      switch( m_direction ) {
//        case North:
//          m_direction = East;
//          to_send_0 = to_send_0 | direction_east;
//          break;
//        case East:
//          m_direction = South;
//          to_send_0 = to_send_0 | direction_south;
//          break;
//        case South:
//          m_direction = West;
//          to_send_0 = to_send_0 | direction_west;
//          break;
//        case West:
//          m_direction = North;
//          to_send_0 = to_send_0 | direction_north;
//          break;
//        default:
//          m_direction = m_direction;
//          break;
//      }
//      break;
//    // U-turn
//    case 1:
//      switch( m_direction ) {
//        case North:
//          m_direction = South;
//          to_send_0 = to_send_0 | direction_south;
//          break;
//        case East:
//          m_direction = West;
//          to_send_0 = to_send_0 | direction_west;
//          break;
//        case South:
//          m_direction = North;
//          to_send_0 = to_send_0 | direction_north;
//          break;
//        case West:
//          m_direction = East;
//          to_send_0 = to_send_0 | direction_east;
//          break;
//        default:
//          m_direction = m_direction;
//          break;
//      }
//       break;
//    // turn left
//    case 2:
//      switch( m_direction ) {
//        case North:
//          m_direction = West;
//          to_send_0 = to_send_0 | direction_west;
//          break;
//        case East:
//          m_direction = South;
//          to_send_0 = to_send_0 | direction_south;
//          break;
//        case South:
//          m_direction = East;
//          to_send_0 = to_send_0 | direction_east;
//          break;
//        case West:
//          m_direction = North;
//          to_send_0 = to_send_0 | direction_north;
//          break;
//        default:
//          m_direction = m_direction;
//          break;
//      }
//      break;
//    // no turn
//    default:
//      m_direction = m_direction;
//      break;
//  }
//}

void backTrack() {
  int prevRow = path[pointer][0];
  int prevCol = path[pointer][0];
  if (prevRow == current[0]) {
    if (prevCol == current[1] - 1) {
      moveWest();
    }
    else {
      moveEast();
    }
  }
  else if (prevCol == current[1]) {
    if (prevRow == current[0] - 1) {
      moveNorth();
    }
    else {
      moveSouth();
    }
  }

  pointer --;
  
}

int detectMicrophone () {
  cli();
  for (int i = 0 ; i < 256 ; i += 2) {
    fft_input[i] = analogRead(A4); // <-- NOTE THIS LINE
    fft_input[i+1] = 0;
  }

  fft_window();
  fft_reorder();
  fft_run();
  fft_mag_log();
  sei();

  //whatever bin number decided goes after the log_out
  // decide threshold by testing the microphone at different distances(?)
  if (fft_log_out[10] > mic_threshold) {
    return 1;
  }
  else {
    return 0;
  }
}


