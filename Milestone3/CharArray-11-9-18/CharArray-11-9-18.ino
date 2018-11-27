#include <Servo.h>
//#include "radio.h"

//#include <SPI.h>
//#include "nRF24L01.h"
//#include "RF24.h"
//#include "printf.h"

//#define LOG_OUT 1 // use the log output function
//#define FFT_N 128 // set to 128 point fft
//#include <FFT.h>

//ARUINO INPUTS
int sensorL = A4;
int sensorR = A5;

int leftWall = A1;
int frontWall = A3;
int rightWall = A2;
//wall selector
//int s2 = 12;
//int s1 = 11;
//int s0 = 10;
////wall sensor
//int walls = A3;

//wall mux
//000 Left
//001 Right
//010 Front


//CALIBRATED GLOBAL VARIABLES
int lineVoltage = 800;
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

int rows = 4; //length along y direction
int cols = 5; //length along x direction
char northWall = 0b00001000;
char eastWall = 0b00000100;
char southWall = 0b00000010;
char westWall = 0b00000001;

char front = 0b00001000;
char right = 0b00000100;
char left = 0b00000001;

enum maze_direction {
  north,
  south,
  east,
  west
};

short maze[5][4] = 
  { {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0},
  };
int moves[60];//rows*cols*3];
int current[2]= {0, 0}; //first is x, second y
int count = 0;
bool backtracking = false;
maze_direction c_direction = north;

void setup() {
  //setting up maze
  maze[0][0] = true;
  
  //wall selects
//  pinMode(s2, OUTPUT);
//  pinMode(s1, OUTPUT);
//  pinMode(s0, OUTPUT);
  
  Serial.begin(9600);
  //printf_begin();
  
  //LED for robot detection
  pinMode(7, OUTPUT);
  servoSetup();

//  radioSetup();
//
//  int mic = 0;
//  while (mic == 0) {
//    mic = detectMicrophone();
//    Serial.println("Waiting for mic");
//  }
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
  delay(500);
  char relativeState = 0; // 1 means can go
  char absoluteState = 0; // 1 means can go

  bool rWall = detectRightWall();
  bool lWall = detectLeftWall();
  bool fWall = detectFrontWall();

  if (!rWall) {
    relativeState = relativeState | right;
  }
  if (!lWall) {
    relativeState = relativeState | left;
  }
  if (!fWall) {
    relativeState = relativeState | front;
  }
//  detectRobot();
//  if (robot == 1) {
//    to_send_0 = to_send_0 | robot_present;
//    relativeState = relativeState & ~front;
//    digitalWrite(7, HIGH);
//    delay(500);
//  }
//  digitalWrite(7, LOW);

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
//  
//  if ((absoluteState & 0b00001000) == 0){
//    to_send_1 = to_send_1 | wall_present_north;
//  }
//  if ((absoluteState & 0b00000100) == 0){
//    to_send_1 = to_send_1 | wall_present_east;
//  }
//  if ((absoluteState & 0b00000010) == 0){
//    to_send_1 = to_send_1 | wall_present_west;
//  }
//  if ((absoluteState & 0b00000001) == 0){
//    to_send_1 = to_send_1 | wall_present_south;
//  }
//  
//  // radio information
//  ping_out( 0b11000000 );
//  delay( 250 );
//  ping_out( to_send_0 );
//  delay( 250 );
//  ping_out( 0b10000000 );
//  delay(250);
//  ping_out( to_send_1 );
//  
//  to_send_0 = 0b00000000;
//  to_send_1 = 0b00000000;

  switch(c_direction) {
    case north :
      current[1] = current[1] + 1;
    case east :
      current[0] = current[0] + 1;
    case south :
      current[1] = current[1] - 1;
    case west :
      current[0] = current[0] - 1;
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
  
  Serial.println("starting turns");
  if ((relativeState & 0b00000100) > 1) {
    turn(1);
    moves[count] = 1; //right
    backtracking = false;
  }
  else if ((relativeState & 0b00001000) > 1) {
    leftservo.write(135);
    rightservo.write(45);
    count++;
    moves[count] = 2; //straight
    backtracking = false;
  }
  else if ((relativeState & 0b00000001) > 1) {
    turn(0);
    moves[count] = 3; //left
    backtracking = false;
  }
  else {
    if (backtracking) {
      backtrack();
    }
    else {
      turn(2);
      backtrack();
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
    Serial.println("end turn");
    leftservo.write(135);
    rightservo.write(45);
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
    turn(0);
  }
  else if (moves[count] == 2)
  {
    moves[count] = 0;
    count = count - 1;
  }
  else if (moves[count] == 3){
    moves[count] = 0;
    count = count - 1;
    turn(1);
  }
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

//void detectRobot() { 
//  //default adc values
//  unsigned int default_timsk = TIMSK0;
//  unsigned int default_adcsra = ADCSRA;
//  unsigned int default_admux = ADMUX;
//  unsigned int default_didr = DIDR0;
//
//  //setup 
//  TIMSK0 = 0; // turn off timer0 for lower jitter
//  ADCSRA = 0xe5; // set the adc to free running mode
//  ADMUX = 0x40; // use adc0
//  DIDR0 = 0x01; // turn off the digital input for adc0
//  
//  cli();
//  for (int i = 0 ; i < 256 ; i += 2) { // save 128 samples
//      while(!(ADCSRA & 0x10)); // wait for adc to be ready
//      ADCSRA = 0xf5; // restart adc
//      byte m = ADCL; // fetch adc data
//      byte j = ADCH;
//      int k = (j << 8) | m; // form into an int
//      k -= 0x0200; // form into a signed int
//      k <<= 6; // form into a 16b signed int
//      fft_input[i] = k; // put real data into even bins
//      fft_input[i+1] = 0; // set odd bins to 0
//    }
//
//  fft_window(); // window the data for better frequency response
//  fft_reorder(); // reorder the data before doing the fft
//  fft_run(); // process the data in the fft
//  fft_mag_log(); // take the output of the fft
//  sei();
//
//  Serial.println(fft_log_out[23]);
//  
//  if (fft_log_out[23] >= 70) {
//    TIMSK0 = default_timsk;
//    ADCSRA = default_adcsra;
//    ADMUX = default_admux;
//    DIDR0 = default_didr;
//    robot = 1;  
//  }
//  else {
//    TIMSK0 = default_timsk;
//    ADCSRA = default_adcsra;
//    ADMUX = default_admux;
//    DIDR0 = default_didr;
//    robot = 0;
//  }
//}
//
//int detectMicrophone () {
//  cli();
//  for (int i = 0 ; i < 256 ; i += 2) {
//    fft_input[i] = analogRead(A4); // <-- NOTE THIS LINE
//    fft_input[i+1] = 0;
//  }
//
//  fft_window();
//  fft_reorder();
//  fft_run();
//  fft_mag_log();
//  sei();
//
//  //whatever bin number decided goes after the log_out
//  // decide threshold by testing the microphone at different distances(?)
//  if (fft_log_out[10] > mic_threshold) {
//    return 1;
//  }
//  else {
//    return 0;
//  }
//}

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
