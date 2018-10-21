// RECEIVER \\

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

int wall_north, wall_east, wall_south, wall_west, treasure_circle, treasure_triangle,
treasure_square, treasure_red, treasure_blue, robot_present, explored, direction_north,
direction_east, direction_south, direction_west;

void setup(void)
{
  //
  // Print preamble
  //

  Serial.begin(9600);
  printf_begin();
  printf("\n\rRF24/examples/GettingStarted/\n\r");
  printf("ROLE: %s\n\r",role_friendly_name[role]);
  printf("*** PRESS 'T' to begin transmitting to the other node\n\r");

  //
  // Setup and configure rf radio
  //
  
  // prints info about radio
  radio.begin();

  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);
  radio.setAutoAck(true);
  // set the channel
  radio.setChannel(0x50);
  // set the power
  // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  radio.setPALevel(RF24_PA_HIGH);
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

  /*if ( role == role_ping_out )
  {
    radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(1,pipes[1]);
  }
  else
  {
    radio.openWritingPipe(pipes[1]);
    radio.openReadingPipe(1,pipes[0]);
  }*/

  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);

  //
  // Start listening
  //

  radio.startListening();

  //
  // Dump the configuration of the rf unit for debugging
  //

  radio.printDetails();
}

char byte1, byte2;

void loop(void)
{
    char* a = pong_back();
    if ( radio.available() ) {
      //Serial.println(*a);
    }
    //Serial.println(*a);
    //Serial.println(*(a+8));
    byte1 = *a;
    byte2 = *( a + 8 );

    // Just need to dereference these
}

unsigned long got_time;
char received_response[2];
char*  pong_back() {
  for (int i = 0; i < 2; i++ ) {
    //
    // Pong back role.  Receive each packet, dump it out, and send it back
    //
      // if there is data ready
      if ( radio.available() )
      {
        // Dump the payloads until we've gotten everything
        bool done = false;
        while (!done)
        {
          // Fetch the payload, and see if this was the last one.
          done = radio.read( &received_response[i], sizeof(char) );
  
          // Spew it
          //printf("Got payload"); //%lu...",got_time);
  
          // Delay just a little bit to let the other unit
          // make the transition to receiver
          delay(20);
  
        }
  
        // First, stop listening so we can talk
        radio.stopListening();
  
        // Send the final one back.
        radio.write( &received_response[i], sizeof(char) );
        printf("Sent response.\n\r");
        Serial.println(int(received_response[0]));
        Serial.println(int(received_response[1]));

        // byte1
        if ( i == 0 ) {
          // North
          if ( received_response[i] >> 2 == 0 ) {
            direction_north = 1;
            direction_east = 0;
            direction_south = 0;
            direction_west = 0;
          }
          // East
          else if ( received_response[i] >> 2 == 1 ) {
            direction_north = 0;
            direction_east = 1;
            direction_south = 0;
            direction_west = 0;
          }
          // South
          else if ( received_response[i] >> 2 == 2 ) {
            direction_north = 0;
            direction_east = 0;
            direction_south = 1;
            direction_west = 0;
          }
          // West
          else {
            direction_north = 0;
            direction_east = 0;
            direction_south = 0;
            direction_west = 1;
          }
          // Explored/unexplored
          // explored
          if ( received_response[i] & 0b00000010 == 2 ) {
            explored = 1;
          }
          // unexplored
          else {
            explored = 0;
          }
          // robot present/not present
          // present
          if ( received_response[i] & 0b00000001 == 1 ) {
            robot_present = 1;
          }
          // not present
          else {
            robot_present = 0;
          }
        }// end byte1 info

        /// byte2
        else {
          // treasure color
          // red
          if ( received_response[i] & 0b01000000 == 64 ) {
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
          if ( ( received_response[i] & 0b00110000 ) >> 4 == 0 ) {
            treasure_circle = 0;
            treasure_triangle = 0;
            treasure_square = 0;
          }
          // circle
          else if ( ( received_response[i] & 0b00110000 ) >> 4 == 1 ) {
            treasure_circle = 1;
            treasure_triangle = 0;
            treasure_square = 0;
          }
          // triangle
          else if ( ( received_response[i] & 0b00110000 ) >> 4 == 2 ) {
            treasure_circle = 0;
            treasure_triangle = 1;
            treasure_square = 0;
          }
          // square
          else if ( ( received_response[i] & 0b00110000 ) >> 4 == 3 ) {
            treasure_circle = 0;
            treasure_triangle = 0;
            treasure_square = 1;
          }

          // wall info
          // north
          if ( received_response[i] & 0b00001111 == 8 ) {
            wall_north = 1;
            wall_east = 0;
            wall_south = 0;
            wall_west = 0;
          }
          // east
          else if ( received_response[i] & 0b00001111 == 4 ) {
            wall_north = 0;
            wall_east = 1;
            wall_south = 0;
            wall_west = 0;
          }
          // south
          else if ( received_response[i] & 0b00001111 == 2 ) {
            wall_north = 0;
            wall_east = 0;
            wall_south = 1;
            wall_west = 0;
          }
          // west
          else {
            wall_north = 0;
            wall_east = 0;
            wall_south = 0;
            wall_west = 1;
          }
        } // end byte2 info
  
        // Now, resume listening so we catch the next packets.
        radio.startListening();
      }
  }
}
