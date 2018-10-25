/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

/**
 * Example for Getting Started with nRF24L01+ radios.
 *
 * This is an example of how to use the RF24 class.  Write this sketch to two
 * different nodes.  Put one of the nodes into 'transmit' mode by connecting
 * with the serial monitor and sending a 'T'.  The ping node sends the current
 * time to the pong node, which responds by sending the value back.  The ping
 * node can then see how long the whole cycle took.
 */

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

//testing variables
int current_location[2] = {0, 1};
int direction[2] = {0,1};
//char to_send[] = {0b00000000,0b00000000};
unsigned char to_send_0 = 0b00000000;
unsigned char to_send_1 = 0b00000000;


//received stuffs
unsigned char byte1, byt2, preamble;
int wall_north, wall_east, wall_south, wall_west, treasure_circle, treasure_triangle,
treasure_square, treasure_red, treasure_blue, robot_pres, explore, north,
east, south, west;

int location[2] = {0, 0};
int byteflag = 0;

void setup(void)
{
  //
  // Print preamble
  //

  Serial.begin(57600);
  printf_begin();
  printf("\n\rRF24/examples/GettingStarted/\n\r");
  printf("ROLE: %s\n\r",role_friendly_name[role]);
  printf("*** PRESS 'T' to begin transmitting to the other node\n\r");

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
  radio.setPayloadSize(8);

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
}

void loop(void)
{
  while (role == role_pong_back ) {
    pong_back();
    change_roles();
  }
  
  current_location[0] = 0;
  current_location[1] = 0;

  for ( int i = 0; i < 9; i++ ) {
      to_send_0 = 0b00000000;
      to_send_1 = 0b00000000;
      
      if ( current_location[0] == 0 && current_location[1] == 4 ) {
        to_send_1 = to_send_1 | treasure_color_red | treasure_present_circle;
        to_send_0 = to_send_0 | robot_present;
      }
      if ( direction[0] == 0 && direction[1] == 1 ) {
        //Serial.println("This");
        //Serial.println(int(to_send[0]));
        to_send_0 = to_send_0 | direction_west;
        //Serial.println(int(to_send[0]));
      }
      to_send_1 = to_send_1 | wall_present_west;
      current_location[0] = current_location[0] + direction[0];
      current_location[1] = current_location[1] + direction[1];
      ping_out( 0b11000000 );
      delay( 250 );
      ping_out( to_send_0 );
      delay( 250 );
      ping_out( 0b10000000 );
      delay(250);
      ping_out( to_send_1 );
      //Serial.println(current_location[1]);
      delay(2000);
    }
}

void ping_out (unsigned char to_send) {
  if (role == role_ping_out ) {
    // First, stop listening so we can talk.
    radio.stopListening();
  
    // Take the time, and send it.  This will block until complete
    unsigned long time = millis();
    printf("Now sending %lu...",to_send);
    bool ok = radio.write( &to_send, sizeof(unsigned char) );
    
    if (ok)
      printf("ok...");
    else
      printf("failed.\n\r");
    
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
      printf("Failed, response timed out.\n\r");
    }
    else
    {
      // Grab the response, compare, and send to debugging spew
      unsigned char got_char;
      radio.read( &got_char, sizeof(unsigned char) );

      if (got_char == to_send) {
        // Spew it
        printf("Got response %lu, round-trip delay: %lu\n\r",got_char,millis()-started_waiting_at);
      }
      else {
        printf("Got WRONG RESPONSE %lu, round-trip delay: %lu\n\r",got_char,millis()-started_waiting_at);
      }
  }
  
    // Try again 1s later
    delay(1000);
  }
}

void pong_back() {
  if ( role == role_pong_back )
  {
    // if there is data ready
    if ( radio.available() )
    {
      // Dump the payloads until we've gotten everything
      unsigned char got_char;
      bool done = false;
      while (!done)
      {
        // Fetch the payload, and see if this was the last one.
        done = radio.read( &got_char, sizeof(unsigned char) );

        // Spew it
        printf("Got payload %lu...",got_char);

        if ( got_char == 0b11000000 ) {
          byteflag = 1;
        }
        else if ( got_char == 0b10000000 ) {
          byteflag = 2;
        }
        else{
          switch ( byteflag ){
            case 1:
              parse_byte_1( got_char );
              break;
            case 2:
              parse_byte_2( got_char );
              break;
            default:
              break;
          }
        }

        // Delay just a little bit to let the other unit
        // make the transition to receiver
        delay(20);

      }

      // First, stop listening so we can talk
      radio.stopListening();

      // Send the final one back.
      radio.write( &got_char, sizeof(unsigned char) );
      printf("Sent response.\n\r");

      // Now, resume listening so we catch the next packets.
      radio.startListening();
    }
  }
}

void change_roles() {
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
}

void parse_byte_1( unsigned char response ) {
  // North
  if ( response >> 2 == 0 ) {
    north = 1;
    east = 0;
    south = 0;
    west = 0;
  }
  // East
  else if ( response >> 2 == 1 ) {
    north = 0;
    east = 1;
    south = 0;
    west = 0;
  }
  // South
  else if ( response >> 2 == 2 ) {
    north = 0;
    east = 0;
    south = 1;
    west = 0;
  }
  // West
  else {
    north = 0;
    east = 0;
    south = 0;
    west = 1;
  }
  
  // Explored/unexplored
  // explored
  if ( response & 0b00000010 == 2 ) {
    explore = 1;
  }
  // unexplored
  else {
    explore = 0;
  }
  
  // robot present/not present
  // present
  if ( response & 0b00000001 == 1 ) {
    robot_pres = 1;
  }
  // not present
  else {
    robot_pres = 0;
  }
}

void parse_byte_2 ( unsigned char response ) {
  // treasure color
  // red
  if ( response & 0b01000000 == 64 ) {
    treasure_red = 1;
    treasure_blue = 0;
  }
  // blue
  else {
    treasure_red = 0;
    treasure_blue = 1;
  }
  
  // treasure shape
  // no treasure
  if ( ( response & 0b00110000 ) >> 4 == 0 ) {
    treasure_circle = 0;
    treasure_triangle = 0;
    treasure_square = 0;
  }
  // circle
  else if ( ( response & 0b00110000 ) >> 4 == 1 ) {
    treasure_circle = 1;
    treasure_triangle = 0;
    treasure_square = 0;
  }
  // triangle
  else if ( ( response & 0b00110000 ) >> 4 == 2 ) {
    treasure_circle = 0;
    treasure_triangle = 1;
    treasure_square = 0;
  }
  // square
  else if ( ( response & 0b00110000 ) >> 4 == 3 ) {
    treasure_circle = 0;
    treasure_triangle = 0;
    treasure_square = 1;
  }

  // wall info
  // north
  wall_north = ( response & 0b00001000 ) >> 3;
  // east
  wall_east = ( response & 0b00000100 ) >> 2;
  // south
  wall_south = ( response & 0b00000010 ) >> 1;
  // west
  wall_west = ( response & 0b00000001 );
}

// vim:cin:ai:sts=2 sw=2 ft=cpp
