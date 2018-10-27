#include <Servo.h>

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
int lineVoltage = 450;
int LRwalls = 195;
int Fwall = 125;
int pause = 1500;

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


//  while (true)
//  {
//    calibrateLineSensors();
//    calibrateWallSensors();
//  }

  //LED for robot detection
  pinMode(7, OUTPUT);
  servoSetup();
  int mic = 0;
  Serial.println("Starting");
  while (mic == 0) {
    mic = detectMicrophone();
    Serial.println("Waiting for mic");
  }
}

/*Use for calculating threshold wall sensor values*/
void calibrateWallSensors()
{
  detectRightWall();
  Serial.print("Right: ");
  Serial.println(read_wallR);

    detectLeftWall();
    Serial.print("Left: ");
    Serial.println(read_wallL);

    detectFrontWall();
    Serial.print("Front: ");
    Serial.println(read_wallF);

    //Serial.print("Analog Read: ");
    //Serial.println(analogRead(walls));
    delay(1000);
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

/*Use for calculating threshold line sensor values*/
void calibrateLineSensors()
{
  Serial.print("Right: ");
  Serial.println(analogRead(sensorR));
  Serial.print("Left: ");
  Serial.println(analogRead(sensorL));
  delay(500);
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

  leftAverage = (readL[0] + readL[1] + readL[2]) / 3;
  rightAverage = (readR[0] + readR[1] + readR[2]) / 3;
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
    delay(500);

    robot = detectRobot();
    robot == 1 ? Serial.println("Robot Detected") : Serial.println("Robot not detected");


    if (robot == 1) {
      digitalWrite(7, HIGH);
      leftservo.write(90);
      rightservo.write(90);
      delay(1000);
      digitalWrite(7, LOW);
    }

    detectFrontWall();
    detectLeftWall();
    detectRightWall();
    read_wallF > Fwall ? Serial.println("Front Wall Detected") : Serial.println("Front Wall not detected");
    read_wallR > LRwalls ? Serial.println("Right Wall Detected") : Serial.println("Right Wall not detected");
    read_wallL > LRwalls ? Serial.println("Left Wall Detected") : Serial.println("Left Wall not detected");

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
      delay(250);
    }
  }
  // Continue turning right
  else if (rightAverage < lineVoltage && leftAverage >= lineVoltage) {
    leftservo.write(135);
    rightservo.write(90);
    Serial.println("Correcting to the Left");
  }
  // Continue turning left
  else if (rightAverage >= lineVoltage && leftAverage < lineVoltage) {
    leftservo.write(90);
    rightservo.write(45);
    Serial.println("Correcting to the Right");
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
    while (!(ADCSRA & 0x10)); // wait for adc to be ready
    ADCSRA = 0xf5; // restart adc
    byte m = ADCL; // fetch adc data
    byte j = ADCH;
    int k = (j << 8) | m; // form into an int
    k -= 0x0200; // form into a signed int
    k <<= 6; // form into a 16b signed int
    fft_input[i] = k; // put real data into even bins
    fft_input[i + 1] = 0; // set odd bins to 0
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

int detectMicrophone () {
  cli();
  for (int i = 0 ; i < 256 ; i += 2) {
    fft_input[i] = analogRead(A4); // <-- NOTE THIS LINE
    fft_input[i + 1] = 0;
  }

  fft_window();
  fft_reorder();
  fft_run();
  fft_mag_log();
  sei();

  //whatever bin number decided goes after the log_out
  // decide threshold by testing the microphone at different distances(?)
  if (fft_log_out[12] > 40) {
    return 1;
    Serial.println("Mic Detected");
  }
  else {
    return 0;
    Serial.println("Mic Not Detected");
  }
}
