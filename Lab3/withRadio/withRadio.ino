#include <Servo.h>
//#include "radio.h"

//#include <SPI.h>
//#include "nRF24L01.h"
//#include "RF24.h"
//#include "printf.h"

#define LOG_OUT 1 // use the log output function
#define FFT_N 128 // set to 128 point fft
#include <FFT.h>

//////RADIO INFORMATION\\\\\\\\
//
// Hardware configuration
//

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10

//RF24 radio(9,10);

//
// Topology
//

// Radio pipe addresses for the 2 nodes to communicate.
//const uint64_t pipes[2] = { 0x0000000004LL, 0x0000000005LL };

//
// Role management
//
// Set up role.  This sketch uses the same software for all the nodes
// in this system.  Doing so greatly simplifies testing.
//

// The various roles supported by this sketch
//typedef enum { role_ping_out = 1, role_pong_back } role_e;

// The debug-friendly names of those roles
//const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};

// The role of the current running sketch
//role_e role = role_ping_out;

// parameters to put into each square
//byte wall_present_north = 0b0001000;
//byte wall_present_east = 0b0000100;
//byte wall_present_south = 0b0000010;
//byte wall_present_west = 0b00000001;
//
//byte treasure_present_circle = 0b00100000;
//byte treasure_present_triangle = 0b01000000;
//byte treasure_present_square = 0b01100000;
//
//byte treasure_color_red = 0b10000000;
//byte treasure_color_blue = 0b00000000;
//
//byte robot_present = 0b00000001;
//
//byte explored = 0b00000010;
//
//byte direction_north = 0b00000000;
//byte direction_east =  0b00000100;
//byte direction_south = 0b00001000;
//byte direction_west =  0b00001100;



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

//wall reads
int read_wallR = 0;
int read_wallL = 0;
int read_wallF = 0;

int robot = 0;

void setup() {
  //wall selects
  pinMode(s2, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s0, OUTPUT);
  
  Serial.begin(9600);
  //printf_begin();
  
  //LED for robot detection
  pinMode(7, OUTPUT);
  servoSetup();

//  radio.begin();
//
//  // optionally, increase the delay between retries & # of retries
//  radio.setRetries(15,15);
//  radio.setAutoAck(true);
//  // set the channel
//  radio.setChannel(0x50);
//  // set the power
//  // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
//  radio.setPALevel(RF24_PA_MIN);
//  //RF24_250KBPS for 250kbs, RF24_1MBPS for 1Mbps, or RF24_2MBPS for 2Mbps
//  radio.setDataRate(RF24_250KBPS);

  // optionally, reduce the payload size.  seems to
  // improve reliability
//  radio.setPayloadSize(8);

  //
  // Open pipes to other nodes for communication
  //

//  radio.openWritingPipe(pipes[0]);
//  radio.openReadingPipe(1,pipes[1]);

  //
  // Start listening
  //

//  radio.startListening();

  //
  // Dump the configuration of the rf unit for debugging
  //

//  radio.printDetails();

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
    // not sure if this stopping is necessary or not
    leftservo.write(90);
    rightservo.write(90);
    
    detectRobot();
    
    while (robot == 1) {
      detectRobot();
      digitalWrite(7, HIGH);
      leftservo.write(90);
      rightservo.write(90);
          
    }
    digitalWrite(7, LOW);

    detectFrontWall();
    detectLeftWall();
    detectRightWall();

    

    // U-turn
    if (read_wallF >= Fwall && read_wallL >= LRwalls && read_wallR >= LRwalls) {
      turn(2);
    }
    // Left Turn
    else if (read_wallF >= Fwall && read_wallL < LRwalls && read_wallR >= LRwalls) {
      turn(0);
    }
    // Right Turn
    else if (read_wallR < LRwalls) {
      turn(1);
    }
    // Go forward
    else {
      leftservo.write(135);
      rightservo.write(45);
    }
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

void detectRightWall() {
  digitalWrite(s2, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s0, LOW);

  read_wallR = analogRead(walls);
}

void detectFrontWall() {
  digitalWrite(s2, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s0, HIGH);

  read_wallF = analogRead(walls);
}

void detectLeftWall() {
  digitalWrite(s2, LOW);
  digitalWrite(s1, HIGH);
  digitalWrite(s0, LOW);

  read_wallL = analogRead(walls);
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

