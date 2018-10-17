// Receiver \\
// Receiver \\
// Receiver \\

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

//
// Hardware configuration
//

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10

RF24 radio(9,10);

//
// Topology
//

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0x0000000004LL, 0x0000000005LL };

//
// Role management
//
// Set up role.  This sketch uses the same software for all the nodes
// in this system.  Doing so greatly simplifies testing.
//

// The various roles supported by this sketch
typedef enum { role_ping_out = 1, role_pong_back } role_e;

// The debug-friendly names of those roles
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};

// The role of the current running sketch
role_e role = role_pong_back;

int done_transmitting;


void setup() {
  //
  // Print preamble
  //

  Serial.begin(57600);
  printf_begin();
  printf("\n\rRF24/examples/GettingStarted/\n\r");
  printf("ROLE: %s\n\r",role_friendly_name[role]);
  printf("*** PRESS 'T' to begin transmitting to the other node\n\r");
  //while ( !Serial.available() ) {};

  //
  // Setup and configure rf radio
  //

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
  //radio.setPayloadSize(8);

  //
  // Open pipes to other nodes for communication
  //

  // This simple sketch opens two pipes for these two nodes to communicate
  // back and forth.
  // Open 'our' pipe for writing
  // Open the 'other' pipe for reading, in position #1 (we can have up to 5 pipes open for reading)

  if ( role == role_ping_out )
  {
    radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(1,pipes[1]);
  }
  else
  {
    radio.openWritingPipe(pipes[1]);
    radio.openReadingPipe(1,pipes[0]);
  }

  //
  // Start listening
  //

  radio.startListening();

  //
  // Dump the configuration of the rf unit for debugging
  //

  radio.printDetails();



  struct square {
      char byte1; // includes explored/unexplored and robot
      char byte2; // includes treasure and walls
    
  } typedef square;
  
  square maze[9][9];
  
}
/*
// parameters to put into each square
#define wall_present_north 0b0001000;
#define wall_present_east 0b0000100;
#define wall_present_south 0b0000010;
#define wall_present_west 0b00000001;

#define treasure_present_circle 0b00100000;
#define treasure_present_triangle 0b01000000;
#define treasure_present_square 0b01100000;

#define treasure_color_red 0b10000000;
#define treasure_color_blue 0b00000000;

#define robot_present 0b00000001;

#define explored 0b00000010;

#define direction_north 0b00000000;
#define direction_east 0b00000100;
#define direction_south 0b00001000;
#define direction_west 0b00001100;
*/

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

byte direction_north = 0b00000000;
byte direction_east = 0b00000100;
byte direction_south = 0b00001000;
byte direction_west = 0b00001100;

// testing variables
int current_location[2] = {0, 0};
int direction[2] = {0,0};
char byte1, byte2;


// example red circle
// byte2 = treasure_color_red | treasure_present_circle


void loop() {
  direction[0] = 0;
  direction[1] = 1;
  current_location[0] = 0;
  current_location[1] = 0;
  
  int i = 0;
  while( i < 9 ) {
    byte1, byte2 = 0b00000000;
    byte1 = byte1 | explored;
    if ( current_location[0] == 0 && current_location[1] == 4 ) {
      byte2 = byte2 | treasure_color_red | treasure_present_circle;
      byte1 = byte1 | robot_present;
    }
    if ( direction[0] == 0 && direction[1] == 1 ) {
      //Serial.println("This");
      //Serial.println(int(byte1));
      byte1 = byte1 | direction_west;
      //Serial.println(int(byte1));
    }
    else {
      byte1 = byte1 | direction_south;
    }

    byte2 = byte2 | wall_present_west;

    //Serial.println(int(byte2));

    
    current_location[0] = current_location[0] + direction[0];
    current_location[1] = current_location[1] + direction[1];
    i += 1;
    done_transmitting = 0;
    radiotransmit( byte1, byte2 );
    while ( !done_transmitting ) {
      Serial.println("Stuck");
      };
    //Serial.println(int(byte1));
    //Serial.println(current_location[1]);
    delay(2000);
  }
}

void radiotransmit( char byte1, char byte2 ) {
//
  // Ping out role.  Repeatedly send the current time
  //

  if (role == role_ping_out)
  {
    // First, stop listening so we can talk.
    radio.stopListening();

    // Take the time, and send it.  This will block until complete
    //unsigned long time = millis();
    printf("Now sending...");
    bool ok = radio.write( &byte1, sizeof(byte1) );

    if (ok)
      printf("ok...\n\r");
    else
      printf("failed.\n\r");

    // Now, continue listening
    radio.startListening();

    // Wait here until we get a response, or timeout (250ms)
    unsigned long started_waiting_at = millis();
    bool timeout = false;
    while ( ! radio.available() && ! timeout )
      if (millis() - started_waiting_at > 1000 )
        timeout = true;

    // Describe the results
    if ( timeout )
    {
      printf("Failed, response timed out.\n\r");
    }
    else
    {
      // Grab the response, compare, and send to debugging spew
      //unsigned long received_byte;
      char received_byte;
      radio.read( &received_byte, sizeof(unsigned long) );

      // Spew it
      //printf("Got response %lu, round-trip delay: %lu\n\r",got_time,millis()-got_time);
      //printf("Got response %c \n", received_byte);
      Serial.println();
      Serial.println(received_byte);
    }

    // Try again 1s later
    delay(1000);
  }

  //
  // Pong back role.  Receive each packet, dump it out, and send it back
  //

  if ( role == role_pong_back )
  {
    // if there is data ready
    if ( radio.available() )
    {
      // Dump the payloads until we've gotten everything
      //unsigned long got_time;
      char received_byte;
      bool done = false;
      while (!done)
      {
        // Fetch the payload, and see if this was the last one.
        done = radio.read( &received_byte, sizeof(byte1) );

        // Spew it
        //printf("Got payload %lu...",got_time);
        //printf("Got payload %c", received_byte);
        Serial.println(int(received_byte));

        // Delay just a little bit to let the other unit
        // make the transition to receiver
        delay(20);

      }

      // First, stop listening so we can talk
      radio.stopListening();

      // Send the final one back.
      radio.write( &received_byte, sizeof(byte1) );
      printf("Sent response.\n\r");

      // Now, resume listening so we catch the next packets.
      radio.startListening();
    }
  }
  //
  // Change roles
  //

  if ( Serial.available() )
  {
    char c = toupper(Serial.read());
    if ( c == 'T' && role == role_pong_back )
    {
      printf("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK\n\r");

      // Become the primary transmitter (ping out)
      role = role_ping_out;
      radio.openWritingPipe(pipes[0]);
      radio.openReadingPipe(1,pipes[1]);
    }
    else if ( c == 'R' && role == role_ping_out )
    {
      printf("*** CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK\n\r");

      // Become the primary receiver (pong back)
      role = role_pong_back;
      radio.openWritingPipe(pipes[1]);
      radio.openReadingPipe(1,pipes[0]);
   }
  }
  done_transmitting = 1;
}

