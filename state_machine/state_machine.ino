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
int lineVoltage = 800;
int LRwalls = 120;
int Fwall = 100;

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
byte wall_present_north = 0b0001000;
byte wall_present_east = 0b0000100;
byte wall_present_south = 0b0000010;
byte wall_present_west = 0b00000001;

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
void setup() {
  Serial.begin(9600);
  servoSetup();
  radioSetup();
  state = FOLLOW_LINE;
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

void turns() {
  if ( rWall && lWall && fWall ) {
    Serial.println("walls on all sides");
    turnLeft();
    turnLeft();
  }
  else if ( rWall && fWall && !lWall ) {
    Serial.println("no left wall");
    turnLeft();
  }
  else if ( !rWall ) {
    Serial.println("no right wall");
    turnRight();
  }
  else {
    forward();
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
          Serial.println("detecting intersection");
          leftservo.write(90);
          rightservo.write(90);
          lWall = detectLeftWall();
          rWall = detectRightWall();
          fWall = detectFrontWall();
        case MOVE : 
          stepPast();
          turns();
        default :
          state = FOLLOW_LINE;
          break;
      }
  }

}
