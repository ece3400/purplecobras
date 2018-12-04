#include <Servo.h>
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

#define LOG_OUT 1 // use the log output function
#define FFT_N 128 // set to 128 point fft
#include <FFT.h>

//ARUINO INPUTS
int sensorL = A4;
int sensorR = A5;
int leftWall = A1;
int frontWall = A3;
int rightWall = A2;
//mic and robot
int mic_ir = A0;
int mic_threshold = 70;

//fft selector
#define s0 4

//CALIBRATED GLOBAL VARIABLES
#define LV 690
#define LRWALLS 150
#define FWALLS 150

//NON-CALIBRATED GLOBAL VARIABLES
Servo rightservo;
Servo leftservo;

bool l_Wall;
bool r_Wall;
bool f_Wall;

//if there is a robot
int robot;
int mic;

//states for the outside loop
enum states {
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
byte wall_present_north = 0b10000000; //front
byte wall_present_east = 0b01000000; //right
byte wall_present_west = 0b00100000; //left

byte Move = 0b00010000;

byte direction_north = 0b00010000;
byte direction_east =  0b00010100;
byte direction_south = 0b00011000;
byte direction_west =  0b00011100;

unsigned char to_send = 0b00000000;

int done_sending = 0;

enum maze_direction {
  North,
  East,
  South,
  West
};

maze_direction m_direction = South;

#define MAZE_X 9
#define MAZE_Y 9

struct node {
  bool visited;
  maze_direction dir;
};

node maze[MAZE_Y][MAZE_X];

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
  for (int i = 0; i < MAZE_Y; i++) {
    for (int j = 0; j <MAZE_X; j++) {
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
  mic = 0;
  robot = 0;
  radioSetup();
  mazeSetup();
  pinMode(s0, OUTPUT);
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  digitalWrite(s0, HIGH);
   while (mic == 0) {
      detectMicrophone();
   }
  digitalWrite(s0, LOW);
  servoSetup();
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

void turnAround() {
  leftAverage = analogRead(sensorL);
  rightAverage = analogRead(sensorR);
  
  leftservo.write(0);
  rightservo.write(0);
  delay(1400);

  while(rightAverage > LV) {
    rightAverage = analogRead(sensorR);
  }
  
}
void turnLeft() {
//  Serial.println("turning left");
  leftAverage = analogRead(sensorL);
  rightAverage = analogRead(sensorR);

  leftservo.write(0);
  rightservo.write(0);
  
  delay(500);
  
  while(rightAverage > LV) {
    rightAverage = analogRead(sensorR);
  }
//  leftservo.write(0);
//  rightservo.write(0);
//  
//  delay(800);
//
//  while(leftAverage > LV && rightAverage > LV) {
//    delay(50);
//    leftAverage = analogRead(sensorL);
//    rightAverage = analogRead(sensorR);
//  }
//  while(leftAverage > LV && rightAverage < LV) {
//    delay(50);
//    leftAverage = analogRead(sensorL);
//    rightAverage = analogRead(sensorR);
//  }
//  while(leftAverage > LV && rightAverage > LV) {
//    delay(50);
//    leftAverage = analogRead(sensorL);
//    rightAverage = analogRead(sensorR);
//  }
//  while (leftAverage < LV && rightAverage > LV) {
//    delay(50);
//    leftAverage = analogRead(sensorL);
//    rightAverage = analogRead(sensorR);
//  }
  
  leftservo.write(90);
  rightservo.write(90);
//  Serial.println("finished turning");
}

void turnRight() {
//  Serial.println("turning right");
  leftAverage = analogRead(sensorL);
  rightAverage = analogRead(sensorR);
  
  leftservo.write(180);
  rightservo.write(180);

  delay(500);
  while(leftAverage > LV) {
    leftAverage = analogRead(sensorL);
  }
//  while(leftAverage > LV && rightAverage > LV) {
//    delay(50);
//    leftAverage = analogRead(sensorL);
//    rightAverage = analogRead(sensorR);
//  }
//  while(leftAverage < LV && rightAverage > LV) {
//    delay(50);
//    leftAverage = analogRead(sensorL);
//    rightAverage = analogRead(sensorR);
//  }
//  while(leftAverage > LV && rightAverage > LV) {
//    delay(50);
//    leftAverage = analogRead(sensorL);
//    rightAverage = analogRead(sensorR);
//  }
//  while (leftAverage > LV && rightAverage < LV) {
//    delay(50);
//    leftAverage = analogRead(sensorL);
//    rightAverage = analogRead(sensorR);
//  }
  
  leftservo.write(90);
  rightservo.write(90);
//  Serial.println("finished turning right");
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

void detectMicrophone () {
  cli();
  for (int i = 0 ; i < 256 ; i += 2) {
    fft_input[i] = analogRead(mic_ir); // <-- NOTE THIS LINE
    fft_input[i+1] = 0;
  }

  fft_window();
  fft_reorder();
  fft_run();
  fft_mag_log();
  sei();
//  Serial.println(fft_log_out[12]);
  //whatever bin number decided goes after the log_out
  // decide threshold by testing the microphone at different distances(?)
//  Serial.println("start");
//    for (byte i = 0 ; i < FFT_N/2 ; i++) { 
//      Serial.println(fft_log_out[i]); // send out the data
//    }
//  
  if (fft_log_out[11] > mic_threshold) {
    mic = 1;
  }
  else {
    mic = 0;
  }
}

int detectRobot() { 
  //default adc values
  unsigned int default_timsk = TIMSK0;
  unsigned int default_adcsra = ADCSRA;
  unsigned int default_admux = ADMUX;
  unsigned int default_didr = DIDR0;

  //setup 
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0
  
  cli();
  for (int i = 0 ; i < 256 ; i += 2) { // save 128 samples
      while(!(ADCSRA & 0x10)); // wait for adc to be ready
      ADCSRA = 0xf5; // restart adc
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      int k = (j << 8) | m; // form into an int
      k -= 0x0200; // form into a signed int
      k <<= 6; // form into a 16b signed int
      fft_input[i] = k; // put real data into even bins
      fft_input[i+1] = 0; // set odd bins to 0
    }

  fft_window(); // window the data for better frequency response
  fft_reorder(); // reorder the data before doing the fft
  fft_run(); // process the data in the fft
  fft_mag_log(); // take the output of the fft
  sei();
//  
//    Serial.println("start");
//    for (byte i = 0 ; i < FFT_N/2 ; i++) { 
//      Serial.println(fft_log_out[i]); // send out the data
//    }
    
  if (fft_log_out[23] >= 160) {
    TIMSK0 = default_timsk;
    ADCSRA = default_adcsra;
    ADMUX = default_admux;
    DIDR0 = default_didr;
    return 1;  
  }
  else {
    TIMSK0 = default_timsk;
    ADCSRA = default_adcsra;
    ADMUX = default_admux;
    DIDR0 = default_didr;
    return 0;
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
          m_direction = North;
          break;
        case South:
          m_direction = East;
          break;
        case West:
          m_direction = South;
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
    turnAround();
//    turnLeft();
//    turnLeft();
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
int getY(int m, int y) {
  if (m == North) {
    return y - 1;
  }
  else if (m == South) {
    return y + 1;
  }
  else {
    return y;
  }
}

int getX(int m, int x) {
  if (m == East) {
    return x + 1;
  }//
  else if (m == West) {
    return x - 1;
  }
  else {
    return x;
  }
}

void updateMaze() {
  maze[current_y][current_x].visited = true;
  if (!backtracking) {
    maze[current_y][current_x].dir = m_direction;
  }
}

void dfs() {
  int right = (m_direction + 1) % 4;
  int left = (m_direction + 3) % 4;
  
//  Serial.print("current x: ");
//  Serial.println(current_x);
//  Serial.print("current y: ");
//  Serial.println(current_y);
  int rx = getX(right, current_x);
  int ry = getY(right, current_y);
  int lx = getX(left, current_x);
  int ly = getY(left, current_y);
  int fx = getX(m_direction, current_x);
  int fy = getY(m_direction, current_y);
//  Serial.print("rx: ");
//  Serial.println(rx);
//  Serial.print("ry: ");
//  Serial.println(ry);
//  Serial.print("lx: ");
//  Serial.println(lx);
//  Serial.print("ly: ");
//  Serial.println(ly);
//  Serial.print("fx: ");
//  Serial.println(fx);
//  Serial.print("fy: ");
//  Serial.println(fy);

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
  if (maze[ry][rx].visited) {
    r_blocked = true;
  }
  if (maze[ly][lx].visited) {
    l_blocked = true;
  }
  if (maze[fy][fx].visited) {
    f_blocked = true;
  }

  if (robot == 1) {
    delay(5000);
    robot = detectRobot();
    if (robot == 1) {
      f_blocked = true;
    }
  }
  
//  if (r_blocked) {
//    Serial.println("r blocked");
//  }
//  if (l_blocked) {
//    Serial.println("l blocked");
//  }
//  if (f_blocked) {
//    Serial.println("f blocked");
//  }
  
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
    int next = (maze[current_y][current_x].dir + 2) % 4;
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
      int ux = getX(next, current_x);
      int uy = getY(next, current_y);
      current_x = ux;
      current_y = uy;
      turnAround();
//      turnRight();
//      turnRight();
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
  ping_out( to_send );
  while ( !done_sending ) {};
}

void updateBytes() {
  switch (m_direction) {
    case North :
      to_send = to_send | direction_north | Move;
      break;
    case South :
      to_send = to_send | direction_south | Move;
      break;
    case East :
      to_send = to_send | direction_east | Move;
      break;
    case West :
      to_send = to_send | direction_west | Move;
      break;
  }

  if (l_Wall) {
    to_send |= wall_present_west; 
  }
  if (r_Wall) {
    to_send |= wall_present_east;
  }
  if (f_Wall) {
    to_send |= wall_present_north;
  }
  
}

void printMaze() {
  for (int i = 0; i < MAZE_Y; i ++ ) {
    for (int j = 0; j < MAZE_X; j++) {
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
//  Serial.println(state);
  switch (state) {
    case FOLLOW_LINE:
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
      break;
    case INTERSECTION:
      switch (action) {
        case DETECT_WALLS :
          stepPast();
          leftservo.write(90);
          rightservo.write(90);
          updateMaze();
//          printMaze();
//          printDirection();
          l_Wall = detectLeftWall();
          r_Wall = detectRightWall();
          f_Wall = detectFrontWall();
          robot = detectRobot();
        case MOVE : 
          updateBytes();
          sendRadio();
          to_send = 0b00000000;
          dfs();
          //turns();
          state = FOLLOW_LINE;
        default :
          state = FOLLOW_LINE;
          break;
      }
  }

}
