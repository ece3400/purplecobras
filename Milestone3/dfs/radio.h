//#include <SPI.h>
//#include "nRF24L01.h"
//#include "RF24.h"
//#include "printf.h"
#ifndef __RADIO_H__
#define __RADIO_H__

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
byte direction_east =  0b00000100;
byte direction_south = 0b00001000;
byte direction_west =  0b00001100;


void radioSetup(void);

void ping_out( unsigned char to_send );

#endif

