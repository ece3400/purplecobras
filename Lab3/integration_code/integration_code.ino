#include <Servo.h>

#define LOG_OUT 1 // use the log output function
#define FFT_N 128 // set to 256 point fft
#include <FFT.h>

//ARUINO INPUTS
int sensorL = A1;
int sensorR = A2;
//int wallR = A3;
//int wallL = A4;
//int wallF = A5;

//wall selector
int s2 = 13;
int s1 = 12;
int s0 = 8;
//wall sensor
int walls = A3

//wall mux
//000 Left
//001 Right
//010 Front


//CALIBRATED GLOBAL VARIABLES
int lineVoltage = 700;
int LRwalls = 195;
int Fwall = 100;
int pause = 1200;

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
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(8, OUTPUT);
  
  Serial.begin(9600);
  
  //LED for robot detection
  pinMode(7, OUTPUT);
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
  // Go forward
  if (rightAverage >= lineVoltage && leftAverage >= lineVoltage) {
    leftservo.write(135);
    rightservo.write(45);
  }
  // intersection
  else if (rightAverage < lineVoltage && leftAverage < lineVoltage) {
    leftservo.write(90);
    rightservo.write(90);
    delay(500);
    Serial.println("before robot detection");
    //adc_Setup();
    robot = detectRobot();
    //adc_Reset();
    
    if (robot == 1) {
      digitalWrite(7, HIGH);
      Serial.println("ROBOT");
      leftservo.write(90);
      rightservo.write(90);
      delay(1000);
      digitalWrite(7, LOW);
    }
    else {
      Serial.println("no robot");
    }
    
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

bool detectLeftWall() {
  digitalWrite(s2, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s0, LOW);
}

//wall mux
//000 Left
//001 Right
//010 Front
bool detectFrontWall() {
  digitalWrite(s2, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s0, LOW);
}

bool detectRightWall() {
  digitalWrite(s2, LOW);
  digitalWrite(s1, LOW);
  digitalWrite(s0, LOW);
}

//void adc_Setup() {
//  //TIMSK0 = 0; // turn off timer0 for lower jitter
//  ADCSRA = 0xe5; // set the adc to free running mode
//  ADMUX = 0x40; // use adc0
//  DIDR0 = 0x01; // turn off the digital input for adc0
//}
//
//void adc_Reset() {
//  //TIMSK0 = default_timsk;
//  ADCSRA = default_adcsra;
//  ADMUX = default_admux;
//  DIDR0 = default_didr;
//}

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

  Serial.println(fft_log_out[23]);
  
  if (fft_log_out[23] >= 70) {
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
