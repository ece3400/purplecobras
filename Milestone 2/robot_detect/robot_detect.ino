#include <Servo.h>

#define LOG_OUT 1 // use the log output function
#define FFT_N 128 // set to 256 point fft
#include <FFT.h>

//ARUINO INPUTS
int sensorL = A1;
int sensorR = A2;
int wallR = A3;
int wallL = A4;
int wallF = A5;

//CALIBRATED GLOBAL VARIABLES
int lineVoltage = 700;

//NON-CALIBRATED GLOBAL VARIABLES
Servo rightservo;
Servo leftservo;

//Averaging the line sensor detection
int readL[3] = {lineVoltage + 100, lineVoltage + 100, lineVoltage + 100};
int readR[3] = {lineVoltage + 100, lineVoltage + 100, lineVoltage + 100};
double leftAverage;
double rightAverage;

//wall reads
int read_wallR;
int read_wallL;
int read_wallF;

//default adc values
int default_timsk = TIMSK0;
int default_adcsra = ADCSRA;
int default_admux = ADMUX;
int default_didr = DIDR0;

void setup() {
  int robot = 0;
  servoSetup();
}

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

  read_wallR = analogRead(wallR);
  read_wallL = analogRead(wallL);
  read_wallF = analogRead(wallF);
}

/*Uses line sensors to have robots follow the line*/
void follow()
{
  if (rightAverage >= lineVoltage && leftAverage >= lineVoltage) {
    leftservo.write(135);
    rightservo.write(45);
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
    //Robot detection
    adc_Setup();
    
    //wall detection
    if (read_wallF >= 135 && read_wallL >= 195 && read_wallR >= 195) {
      turn(true);
      turn(true);
    }
    else if (read_wallF >= 135 && read_wallL < 195 && read_wallR >= 195) {
      turn(false);
    }
    else if (read_wallR < 195) {
      turn(true);
    }
  }
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

void adc_Setup() {
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0
}

void adc_Reset() {
  TIMSK0 = default_timsk;
  ADCSRA = default_adcsra;
  ADMUX = default_admux;
  DIDR0 = default_didr;
}

int detectRobot() {
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

  if (fft_log_out[23] >= 100) {
    return 1;  
  }
  else {
    return 0;
  }
}

