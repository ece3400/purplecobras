// TRANSMITTER \\

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
char to_send[2] = {0b00000000,0b00000000};


void loop(void)
{
    current_location[0] = 0;
    current_location[1] = 0;

    for ( int i = 0; i < 9; i++ ) {
      to_send[0] = 0b00000000;
      to_send[1] = 0b00000000;
      
      if ( current_location[0] == 0 && current_location[1] == 4 ) {
        to_send[1] = to_send[1] | treasure_color_red | treasure_present_circle;
        to_send[0] = to_send[0] | robot_present;
      }
      if ( direction[0] == 0 && direction[1] == 1 ) {
        //Serial.println("This");
        //Serial.println(int(to_send[0]));
        to_send[0] = to_send[0] | direction_west;
        //Serial.println(int(to_send[0]));
      }
      to_send[1] = to_send[1] | wall_present_west;
      current_location[0] = current_location[0] + direction[0];
      current_location[1] = current_location[1] + direction[1];
      pong_out( to_send );
      //Serial.println(current_location[1]);
      delay(2000);
    }
}

void pong_out( char to_send[2] ) {
  for ( int i = 0; i < 2; i++ ) {
    //
    // Ping out role.  Repeatedly send the current time
    //
      // First, stop listening so we can talk.
      radio.stopListening();
  
      // Take the time, and send it.  This will block until complete
      unsigned long time = millis();
      printf("Now sending" ); //%lu...",time);
      bool ok = radio.write( &(to_send[i]), sizeof(char) );
  
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
        char got_char[2];
        radio.read( &got_char[i], sizeof(char) );
  
        // Spew it
        printf("Got response");//%lu, round-trip delay: %lu\n\r",got_time,millis()-got_time);
      }
  
      // Try again 1s later
      delay(1000);
  }
}
  
