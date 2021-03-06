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

//testing variables
//int current_location[2] = {0,0};
int current_location_rec[2] = {0, -1};
int direction[2] = {0,1};
//char to_send[] = {0b00000000,0b00000000};
unsigned char to_send = 0b00000000;

int location[2] = {0, 0};

void setup(void)
{
  //
  // Print preamble
  //

  Serial.begin(9600);
  printf_begin();

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

      radio.openWritingPipe(pipes[1]);
      radio.openReadingPipe(1,pipes[0]);

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
  //Serial.println("0,1, west=false");
  while (role == role_pong_back ) {
    pong_back();
    //change_roles();
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
        //printf("Got payload %lu...",got_char);

        parse_byte( got_char );

        // Delay just a little bit to let the other unit
        // make the transition to receiver
        delay(20);

      }

      // First, stop listening so we can talk
      radio.stopListening();

      // Send the final one back.
      radio.write( &got_char, sizeof(unsigned char) );
      //printf("Sent response.\n\r");

      // Now, resume listening so we catch the next packets.
      radio.startListening();
    }
  }
}

void parse_byte( unsigned char response ) {
  String Direction_str = "";
  int Direction;
  Direction = ( response & (0b00001100) ) >> 2;

  int Move;
  Move = ( response & (0b00010000) ) >> 4;
  
  if ( Move ) {
    switch(Direction){
      case 0:
        // decrement y value because going north
        current_location_rec[1] = current_location_rec[1] - 1;
        Direction_str = Direction_str + String(current_location_rec[1]) + "," + String(current_location_rec[0]);
        break;
      case 1:
        // increment x value because going east
        current_location_rec[0] = current_location_rec[0] + 1;
        Direction_str = Direction_str + String(current_location_rec[1]) + "," + String(current_location_rec[0]);
        break;
      case 2:
        // increment the y value because going south
        current_location_rec[1] = current_location_rec[1] + 1;
        Direction_str = Direction_str + String(current_location_rec[1]) + "," + String(current_location_rec[0]);
        break;
       case 3:
         // decrement x value because going west
         current_location_rec[0] = current_location_rec[0] - 1;
         Direction_str = Direction_str + String(current_location_rec[1]) + "," + String(current_location_rec[0]);
         break;
       default:
         // Do write the current location without changing location
         Direction_str = Direction_str + String(current_location_rec[1]) + "," + String(current_location_rec[0]);
      }
    }
    else {
      Direction_str = Direction_str + String(current_location_rec[1]) + "," + String(current_location_rec[0]);
    }

    String Wall_str = "";
    int North_wall,East_wall, South_wall, West_wall;
    North_wall = ( response & (0b10000000) ) >> 7;
    East_wall = ( response & (0b01000000) ) >> 6;
    West_wall = ( response & (0b00100000) ) >> 5;

    switch(Direction){
      // north
      case 0:
        if ( North_wall ) Wall_str += ",north=True";
        if ( East_wall ) Wall_str += ",east=True";
        if ( South_wall )Wall_str += ",south=True" ;
        if ( West_wall ) Wall_str += ",west=True";
        if ( ( North_wall | East_wall | South_wall | West_wall ) == 0 ) Wall_str += ",north=False";
        break;
      // east
      case 1:
        if ( North_wall ) Wall_str += ",east=True";
        if ( East_wall ) Wall_str += ",south=True";
        if ( South_wall )Wall_str += ",west=True" ;
        if ( West_wall ) Wall_str += ",north=True";
        if ( ( North_wall | East_wall | South_wall | West_wall ) == 0 ) Wall_str += ",north=False";
        break;
      // south
      case 2:
        if ( North_wall ) Wall_str += ",south=True";
        if ( East_wall ) Wall_str += ",west=True";
        if ( South_wall )Wall_str += ",north=True" ;
        if ( West_wall ) Wall_str += ",east=True";
        if ( ( North_wall | East_wall | South_wall | West_wall ) == 0 ) Wall_str += ",north=False";
        break;
      // west
       case 3:
        if ( North_wall ) Wall_str += ",west=True";
        if ( East_wall ) Wall_str += ",north=True";
        if ( South_wall )Wall_str += ",east=True" ;
        if ( West_wall ) Wall_str += ",south=True";
        if ( ( North_wall | East_wall | South_wall | West_wall ) == 0 ) Wall_str += ",north=False";
         break;
       default:
         break;
      }
    
    String to_Gui = "";
    to_Gui += Direction_str + Wall_str;
    Serial.println(to_Gui);
}

// vim:cin:ai:sts=2 sw=2 ft=cpp
