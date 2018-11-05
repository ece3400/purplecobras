#include <Servo.h>
#include "radio.h"

//#include <SPI.h>
//#include "nRF24L01.h"
//#include "RF24.h"
//#include "printf.h"

#define LOG_OUT 1 // use the log output function
#define FFT_N 128 // set to 128 point fft
#include <FFT.h>

//ARUINO INPUTS
int sensorL = A1;
int sensorR = A2;

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

int robot = 0;
// radio information
unsigned char to_send_0 = 0b00000000;
unsigned char to_send_1 = 0b00000000;

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
int current[]= {0, 0}; //first is rows, second columns
maze_direction c_direction = North;

void setup() {
  //setting up maze
  maze[0][0] = true;
  
  //wall selects
  pinMode(s2, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s0, OUTPUT);
  
  Serial.begin(9600);
  //printf_begin();
  
  //LED for robot detection
  pinMode(7, OUTPUT);
  servoSetup();

  radioSetup();

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

  bool nGood = true;
  bool sGood = true;
  bool wGood = true;
  bool eGood = true;
  bool fGood = true;
  bool lGood = true;
  bool rGood = true;
  bool rWall = detectRightWall();
  bool lWall = detectLeftWall();
  bool fWall = detectFrontWall();
   
  detectRobot();
  switch(c_direction) { //update current location
    case North :
      nGood = false;
    case East :
      eGood = false;
    case South :
      sGood = false;
    case West :
      wGood = false;
  }
  if (!nGood){
    to_send_1 = to_send_1 | wall_present_north;
  }
  if (!eGood){
    to_send_1 = to_send_1 | wall_present_east;
  }
  if (!wGood){
    to_send_1 = to_send_1 | wall_present_west;
  }
  if (!sGood){
    to_send_1 = to_send_1 | wall_present_south;
  }
  
  // radio information
  ping_out( 0b11000000 );
    delay( 250 );
    ping_out( to_send_0 );
    delay( 250 );
    ping_out( 0b10000000 );
    delay(250);
    ping_out( to_send_1 );

  to_send_0 = 0b00000000;
  to_send_1 = 0b00000000;
  
  while (robot == 1) {
    detectRobot();
    to_send_0 = to_send_0 | robot_present;
    digitalWrite(7, HIGH);
    leftservo.write(90);
    rightservo.write(90);
        
  }
  digitalWrite(7, LOW);

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
  
  switch (c_direction) {
    case North :
      nGood &= !fWall;
      eGood &= !rWall;
      wGood &= !lWall;
    case East :
      nGood &= !lWall;
      eGood &= !fWall;
      sGood &= !rWall;
    case South :
      eGood &= !lWall;
      wGood &= !rWall;
      sGood &= !fWall;
    case West :
      nGood &= !rWall;
      wGood &= !fWall;
      sGood &= !lWall;
  }
  nGood &= !maze[current[0]+1][current[1]];
  eGood &= !maze[current[0]][current[1]+1];
  wGood &= !maze[current[0]][current[1]-1];
  sGood &= !maze[current[0]-1][current[1]];
  maze[current[0]][current[1]] = true;

  if (c_direction == North){
    fGood = nGood;
    rGood = eGood;
    lGood = wGood;
  }
  else if (c_direction == East){
    fGood = eGood;
    rGood = sGood;
    lGood = nGood;
  }
  else if (c_direction == South){
    fGood = sGood;
    rGood = wGood;
    lGood = eGood;
  }
  else if (c_direction == West){
    fGood = wGood;
    rGood = nGood;
    lGood = sGood;
  }
  
  
  
  if (rGood) {
    
  }
  else if (fGood) {
    
  }
  else if (lGood) {
    
  }
  else {
    //backtrack(); //need to write method to return to node with nonvisited branch
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
    leftservo.write(135);
    rightservo.write(45);
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

void detectRobot() { 
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

  Serial.println(fft_log_out[23]);
  
  if (fft_log_out[23] >= 70) {
    TIMSK0 = default_timsk;
    ADCSRA = default_adcsra;
    ADMUX = default_admux;
    DIDR0 = default_didr;
    robot = 1;  
  }
  else {
    TIMSK0 = default_timsk;
    ADCSRA = default_adcsra;
    ADMUX = default_admux;
    DIDR0 = default_didr;
    robot = 0;
  }
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


void change_direction(int how_many_turn) {
  switch( how_many_turn ) {
    // turn right
    case 0:
      switch( m_direction ) {
        case North:
          m_direction = East;
          to_send_0 = to_send_0 | direction_east;
          break;
        case East:
          m_direction = South;
          to_send_0 = to_send_0 | direction_south;
          break;
        case South:
          m_direction = West;
          to_send_0 = to_send_0 | direction_west;
          break;
        case West:
          m_direction = North;
          to_send_0 = to_send_0 | direction_north;
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
          to_send_0 = to_send_0 | direction_south;
          break;
        case East:
          m_direction = West;
          to_send_0 = to_send_0 | direction_west;
          break;
        case South:
          m_direction = North;
          to_send_0 = to_send_0 | direction_north;
          break;
        case West:
          m_direction = East;
          to_send_0 = to_send_0 | direction_east;
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
          to_send_0 = to_send_0 | direction_west;
          break;
        case East:
          m_direction = South;
          to_send_0 = to_send_0 | direction_south;
          break;
        case South:
          m_direction = East;
          to_send_0 = to_send_0 | direction_east;
          break;
        case West:
          m_direction = North;
          to_send_0 = to_send_0 | direction_north;
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
