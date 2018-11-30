#include <Servo.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

//ARUINO INPUTS
int sensorL = A4;
int sensorR = A5;
int leftWall = A1;
int frontWall = A3;
int rightWall = A2;

//CALIBRATED GLOBAL VARIABLES
#define LV 400
#define LRWALLS 150
#define FWALLS 150

//NON-CALIBRATED GLOBAL VARIABLES
Servo rightservo;
Servo leftservo;

bool l_Wall;
bool r_Wall;
bool f_Wall;

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

//radio information
RF24 radio(9,10);

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0x0000000004LL, 0x0000000005LL };

// The various roles supported by this sketch
typedef enum { role_ping_out = 1, role_pong_back } role_e;

// The debug-friendly names of those roles
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};

// The role of the current running sketch
role_e role = role_ping_out;

// parameters to put into each square
byte wall_present_north = 0b0001000; //front
byte wall_present_east = 0b0000100; //right
//byte wall_present_south = 0b0000010;
byte wall_present_west = 0b00000001; //left

byte treasure_present_circle = 0b00100000;
byte treasure_present_triangle = 0b01000000;
byte treasure_present_square = 0b01100000;

byte treasure_color_red = 0b10000000;
byte treasure_color_blue = 0b00000000;

byte robot_present = 0b00000001;

byte explored = 0b00000010;

byte direction_north = 0b00010000;
byte direction_east =  0b00010100;
byte direction_south = 0b00011000;
byte direction_west =  0b00011100;

unsigned char to_send_0 = 0b00000000;
unsigned char to_send_1 = 0b00000000;

int done_sending = 0;

enum maze_direction {
  North,
  East,
  South,
  West
};

maze_direction m_direction = North;

#define MAZE_X 9
#define MAZE_Y 9

struct node {
  bool visited;
  maze_direction dir;
};

node maze[MAZE_X][MAZE_Y];

int current_x = 0;
int current_y = 0;
bool backtracking = false;

/*Sets up servos*/
void servoSetup()
{
  rightservo.attach(3);
  leftservo.attach(5);
  rightservo.write(90);
  leftservo.write(90);
}

/*sets up radio */
void radioSetup() 
{
  radio.begin();

  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);
  radio.setAutoAck(true);
  // set the channel
  radio.setChannel(0x50);
  // set the power
  // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  radio.setPALevel(RF24_PA_MIN);
  //RF24_250KBPS for 250kbs, RF24_1MBPS for 1Mbps, or RF24_2MBPS for 2Mbps
  radio.setDataRate(RF24_250KBPS);

  // optionally, reduce the payload size.  seems to
  // improve reliability
  radio.setPayloadSize(8);

  // Open pipes to other nodes for communication
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);

  // Start listening
  radio.startListening();

  // Dump the configuration of the rf unit for debugging
  radio.printDetails();
}

void mazeSetup() {
  for (int i = 0; i < MAZE_X; i++) {
    for (int j = 0; j <MAZE_Y; j++) {
      if (i == 0 && j == 0) {
        maze[i][j].visited = true;
      }
      else {
        maze[i][j].visited = false;
      }
    }
  }
}

void setup() {
  Serial.begin(9600);
  servoSetup();
  radioSetup();
  mazeSetup();
  pinMode(LED_BUILTIN, OUTPUT);
  state = FOLLOW_LINE;
}

int readL[3] = {LV + 100, LV + 100, LV + 100};
int readR[3] = {LV + 100, LV + 100, LV + 100};
double leftAverage;
double rightAverage;

void sample()
{
//  readR[0] = readR[1];
//  readR[1] = readR[2];
//  readR[2] = analogRead(sensorR);
//  readL[0] = readL[1];
//  readL[1] = readL[2];
//  readL[2] = analogRead(sensorL);
  int left_1 = analogRead(sensorL);
  int right_1 = analogRead(sensorR);
  int left_2 = analogRead(sensorL);
  int right_2 = analogRead(sensorR);
  int left_3 = analogRead(sensorL);
  int right_3 = analogRead(sensorR);
  
  leftAverage = (left_1 + left_2 + left_3)/3;
  rightAverage = (right_1 + right_2 + right_3)/3;
}

void follow() {
    sample();
  if (rightAverage >= LV && leftAverage >= LV) {
    forward();
    state = FOLLOW_LINE;
  }
  else if (rightAverage < LV && leftAverage < LV) {
    state = INTERSECTION;
  }
  // Continue turning right
  else if (rightAverage < LV && leftAverage >= LV) {
    leftservo.write(135);
    rightservo.write(90);
    state = FOLLOW_LINE;
  }
  // Continue turning left
  else if (rightAverage >= LV && leftAverage < LV) {
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
  
  while (leftAverage < LV && rightAverage < LV) {
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
  Serial.println("turning left");
  leftAverage = analogRead(sensorL);
  rightAverage = analogRead(sensorR);
  
  leftservo.write(0);
  rightservo.write(0);
  
  while(leftAverage > LV && rightAverage > LV) {
    delay(50);
    leftAverage = analogRead(sensorL);
    rightAverage = analogRead(sensorR);
  }
  while(leftAverage > LV && rightAverage < LV) {
    delay(50);
    leftAverage = analogRead(sensorL);
    rightAverage = analogRead(sensorR);
  }
  while(leftAverage > LV && rightAverage > LV) {
    delay(50);
    leftAverage = analogRead(sensorL);
    rightAverage = analogRead(sensorR);
  }
  while (leftAverage < LV && rightAverage > LV) {
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
  
  while(leftAverage > LV && rightAverage > LV) {
    delay(50);
    leftAverage = analogRead(sensorL);
    rightAverage = analogRead(sensorR);
  }
  while(leftAverage < LV && rightAverage > LV) {
    delay(50);
    leftAverage = analogRead(sensorL);
    rightAverage = analogRead(sensorR);
  }
  while(leftAverage > LV && rightAverage > LV) {
    delay(50);
    leftAverage = analogRead(sensorL);
    rightAverage = analogRead(sensorR);
  }
  while (leftAverage > LV && rightAverage < LV) {
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
  if (analogRead(rightWall) >= LRWALLS) {
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
  if (analogRead(frontWall) >= FWALLS) {
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
  
  if (analogRead(leftWall) >= LRWALLS) {
    return true;
  }
  else {
    return false;
  }
}

void change_direction(int how_many_turn) {
  switch( how_many_turn ) {
    // turn right
    case 0:
      switch( m_direction ) {
        case North:
          m_direction = East;
          break;
        case East:
          m_direction = South;
          break;
        case South:
          m_direction = West;
          break;
        case West:
          m_direction = North;
          break;
        default:
          m_direction = m_direction;
          break;
      }
      break;
    // U-turn
    case 1:
      switch( m_direction ) {
        case North:
          m_direction = South;
          break;
        case East:
          m_direction = West;
          break;
        case South:
          m_direction = North;
          break;
        case West:
          m_direction = East;
          break;
        default:
          m_direction = m_direction;
          break;
      }
       break;
    // turn left
    case 2:
      switch( m_direction ) {
        case North:
          m_direction = West;
          break;
        case East:
          m_direction = South;
          break;
        case South:
          m_direction = East;
          break;
        case West:
          m_direction = North;
          break;
        default:
          m_direction = m_direction;
          break;
      }
      break;
    // no turn
    default:
      m_direction = m_direction;
      break;
  }
}

void turns() {
  if ( r_Wall && l_Wall && f_Wall ) {
    change_direction(1);
    turnLeft();
    turnLeft();
  }
  else if ( r_Wall && f_Wall && !l_Wall ) {
    change_direction(2);
    turnLeft();
  }
  else if ( !r_Wall ) {
    change_direction(0);
    turnRight();
  }
  else {
    forward();
  }
}

//CHANGE THIS BASED OFF OF GUI 
//for now North decrement row, west decrement col
int getX(int m) {
  if (m == North) {
    return current_x + 1;
  }
  else if (m == South) {
    return current_x - 1;
  }
  else {
    return current_x;
  }
}

int getY(int m) {
  if (m == East) {
    return current_y + 1;
  }//
  else if (m == West) {
    return current_y - 1;
  }
  else {
    return current_y;
  }
}

void updateMaze() {
  maze[current_x][current_y].visited = true;
  if (!backtracking) {
    maze[current_x][current_y].dir = m_direction;
  }
}

void dfs() {
  int right = (m_direction + 1) % 4;
  int left = (m_direction + 3) % 4;
  
  int rx = getX(right);
  int ry = getY(right);
  int lx = getX(left);
  int ly = getY(left);
  int fx = getX(m_direction);
  int fy = getY(m_direction);

  bool r_blocked = false;
  bool l_blocked = false;
  bool f_blocked = false;

  //if there are walls blocking
  if ( r_Wall ) {
    r_blocked = true;
  }
  if ( l_Wall ) {
    l_blocked = true;
  }
  if ( f_Wall ) {
    f_blocked = true;
  }

  //check surrounding nodes to see if they have been visited
  if (maze[rx][ry].visited) {
    r_blocked = true;
  }
  if (maze[lx][ly].visited) {
    l_blocked = true;
  }
  if (maze[fx][fy].visited) {
    f_blocked = true;
  }

  if (r_blocked) {
    Serial.println("r blocked");
  }
  if (l_blocked) {
    Serial.println("l blocked");
  }
  if (f_blocked) {
    Serial.println("f blocked");
  }
  
  if (!r_blocked) {
    backtracking = false;
    current_x = rx;
    current_y = ry;
    turnRight();
    change_direction(0);
  }
  else if (!f_blocked) {
    backtracking = false;
    current_x = fx;
    current_y = fy;
    forward();
  }
  else if (!l_blocked) {
    backtracking = false;
    current_x = lx;
    current_y = ly;
    turnLeft();
    change_direction(2);
  }
  else {
    //reversing the direction
    backtracking = true;
    int next = (maze[current_x][current_y].dir + 2) % 4;
    if (next == right) {
      current_x = rx;
      current_y = ry;
      turnRight();
      change_direction(0);
    }
    else if (next == left) {
      current_x = lx;
      current_y = ly;
      turnLeft();
      change_direction(2);
    }
    else if (next == m_direction) {
      current_x = fx;
      current_y = fy;
      forward();
    }
    else {
      int ux = getX(next);
      int uy = getY(next);
      current_x = ux;
      current_y = uy;
      turnRight();
      turnRight();
      change_direction(1);
    }
  }
}
void ping_out (unsigned char to_send) {
  done_sending = 0;
  if (role == role_ping_out ) {
    // First, stop listening so we can talk.
    radio.stopListening();
  
    // Take the time, and send it.  This will block until complete
    unsigned long time = millis();
    //printf("Now sending %lu...",to_send);
    bool ok = radio.write( &to_send, sizeof(unsigned char) );
    
    if (ok) {
      //printf("ok...");
    }
    else{
      //printf("failed.\n\r");
    }
    
    // Now, continue listening
    radio.startListening();
    
    // Wait here until we get a response, or timeout (250ms)
    unsigned long started_waiting_at = millis();
    bool timeout = false;
    while ( ! radio.available() && ! timeout )
    if (millis() - started_waiting_at > 200 )
      timeout = true;
    
    // Describe the results
    if ( timeout )
    {
      //printf("Failed, response timed out.\n\r");
    }
    else
    {
      // Grab the response, compare, and send to debugging spew
      unsigned char got_char;
      radio.read( &got_char, sizeof(unsigned char) );

      if (got_char == to_send) {
        // Spew it
        //printf("Got response %lu, round-trip delay: %lu\n\r",got_char,millis()-started_waiting_at);
      }
      else {
        //printf("Got WRONG RESPONSE %lu, round-trip delay: %lu\n\r",got_char,millis()-started_waiting_at);
      }
  }
  
    // Try again 1s later
    delay(1000);
  }
  done_sending = 1;
}

void sendRadio() {
  ping_out( 0b11000000 );
  while ( !done_sending ) {};
  //delay( 250 );
  ping_out( to_send_0 );
  while ( !done_sending ) {};
  //delay( 400 );
  ping_out( 0b10000000 );
  while ( !done_sending ) {};
  //delay(250);
  ping_out( to_send_1 );
  while ( !done_sending ) {};
}

void updateBytes() {
  switch (m_direction) {
    case North :
      to_send_0 = to_send_0 | direction_north;
      break;
    case South :
      to_send_0 = to_send_0 | direction_south;
      break;
    case East :
      to_send_0 = to_send_0 | direction_east;
      break;
    case West :
      to_send_0 = to_send_0 | direction_west;
      break;
  }

  if (l_Wall) {
    to_send_1 |= wall_present_west; 
  }
  if (r_Wall) {
    to_send_1 |= wall_present_east;
  }
  if (f_Wall) {
    to_send_1 |= wall_present_north;
  }
  
}

void printMaze() {
  for (int i = 0; i < MAZE_X; i ++ ) {
    for (int j = 0; j < MAZE_Y; j++) {
      if (maze[i][j].visited) {
        Serial.print(1);
      }
      else {
        Serial.print(0);
      }
    }
    Serial.println();
  }
}

void printDirection() {
  if (m_direction == North) {
    Serial.println("N");
  }
  else if (m_direction == East) {
    Serial.println("E");
  }
  else if (m_direction == South) {
    Serial.println("S");
  }
  else{
    Serial.println("W");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  switch (state) {
    case FOLLOW_LINE:
      sample();
      Serial.println("hi");
      if (rightAverage >= LV && leftAverage >= LV) {
        forward();
        state = FOLLOW_LINE;
      }
      else if (rightAverage < LV && leftAverage < LV) {
        state = INTERSECTION;
      }
      // Continue turning right
      else if (rightAverage < LV && leftAverage >= LV) {
        leftservo.write(135);
        rightservo.write(90);
        state = FOLLOW_LINE;
      }
      // Continue turning left
      else if (rightAverage >= LV && leftAverage < LV) {
        leftservo.write(90);
        rightservo.write(45);
        state = FOLLOW_LINE;
      }
      break;
    case INTERSECTION:
      switch (action) {
        case DETECT_WALLS :
          leftservo.write(90);
          rightservo.write(90);
          updateMaze();
          printMaze();
          printDirection();
          l_Wall = detectLeftWall();
          r_Wall = detectRightWall();
          f_Wall = detectFrontWall();
        case MOVE : 
//          sendRadio();
//          to_send_0 = 0b00000000;
//          to_send_1 = 0b00000000;
          stepPast();
          dfs();
          //turns();
          state = FOLLOW_LINE;
        default :
          state = FOLLOW_LINE;
          break;
      }
  }

}
