//#include <Wire.h>
//
//#define OV7670_I2C_ADDRESS 0x21 /*TODO: write this in hex (eg. 0xAB) */
//
//
/////////// Main Program //////////////
//void setup() {
//  Wire.begin();
//  Serial.begin(9600);
//  // TODO: READ KEY REGISTERS
//  int resetRegInit = read_register_value(0x12);
//  int scalingRegInit1 = read_register_value(0x0C);
//  int scalingRegInit2 = read_register_value(0x3E);
//  int clkRegInit = read_register_value(0x11);
//  int testRegInit1 = read_register_value(0x70);
//  int testRegInit2 = read_register_value(0x71);
//  int formatRegInit = read_register_value(0x40);
//  
//  delay(100);
//
//  Serial.println("Desired Values:");
//  Serial.println(resetRegInit & 0b01111111);
//  Serial.println(scalingRegInit1 | 0b00001000);
//  Serial.println(scalingRegInit2 | 0b00001000);
//  Serial.println((formatRegInit | 0b00010000) & 0b11101111);
//  Serial.println(clkRegInit | 0b01000000);
//  Serial.println((clkRegInit | 0b10000000) & 0b01111111);
//  Serial.println("End Desired Values");
//  
//  // TODO: WRITE KEY REGISTERS
//  OV7670_write_register(0x12, resetRegInit | 0b10000000); //reset registers
//  resetRegInit = read_register_value(0x12);
//  scalingRegInit1 = read_register_value(0x0C);
//  scalingRegInit2 = read_register_value(0x3E);
//  clkRegInit = read_register_value(0x11);
//  testRegInit1 = read_register_value(0x70);
//  testRegInit2 = read_register_value(0x71);
//  formatRegInit = read_register_value(0x40);
//  Serial.println("Reset Values:");
//  Serial.println(resetRegInit & 0b01111111);
//  Serial.println(scalingRegInit1 | 0b00001000);
//  Serial.println(scalingRegInit2 | 0b00001000);
//  Serial.println((formatRegInit | 0b00010000) & 0b11101111);
//  Serial.println(clkRegInit | 0b01000000);
//  Serial.println((clkRegInit | 0b10000000) & 0b01111111);
//  Serial.println("End Reset Values");
//  
//  OV7670_write_register(0x12, resetRegInit & 0b01111111); //registers back to normal mode
//  OV7670_write_register(0x0C, scalingRegInit1 | 0b00001000); //enable scaling
//  OV7670_write_register(0x3E, scalingRegInit2 | 0b00001000); //enable scaling with custom resolution
//  OV7670_write_register(0x40, (formatRegInit | 0b00010000) & 0b11101111); //enable scaling with custom resolution
//  OV7670_write_register(0x11, clkRegInit | 0b01000000); //use external clock
//  OV7670_write_register(0x70, clkRegInit | 0b10000000); //color bar test
//  OV7670_write_register(0x70, clkRegInit & 0b01111111); //color bar test
//
//
//  set_color_matrix();
//
//  
//  read_key_registers();
//}
//
//void loop(){
//  //read_key_registers();
//  delay(1000);
//}
//
//
/////////// Function Definition //////////////
//void read_key_registers(){
//  /*TODO: DEFINE THIS FUNCTION*/
//  Serial.println(read_register_value(0x12));
//  Serial.println(read_register_value(0x0C));
//  Serial.println(read_register_value(0x3E));
//  Serial.println(read_register_value(0x40));
//  Serial.println(read_register_value(0x11));
//  Serial.println(read_register_value(0x70));
//}
//
//byte read_register_value(int register_address){
//  byte data = 0;
//  Wire.beginTransmission(OV7670_I2C_ADDRESS);
//  Wire.write(register_address);
//  Wire.endTransmission();
//  Wire.requestFrom(OV7670_I2C_ADDRESS,1);
//  while(Wire.available()<1);
//  data = Wire.read();
//  return data;
//}
//
//String OV7670_write(int start, const byte *pData, int size){
//    int n,error;
//    Wire.beginTransmission(OV7670_I2C_ADDRESS);
//    n = Wire.write(start);
//    if(n != 1){
//      return "I2C ERROR WRITING START ADDRESS";   
//    }
//    n = Wire.write(pData, size);
//    if(n != size){
//      return "I2C ERROR WRITING DATA";
//    }
//    error = Wire.endTransmission(true);
//    if(error != 0){
//      return String(error);
//    }
//    return "no errors :)";
// }
//
//String OV7670_write_register(int reg_address, byte data){
//  return OV7670_write(reg_address, &data, 1);
// }
//
//void set_color_matrix(){
//    OV7670_write_register(0x4f, 0x80);
//    OV7670_write_register(0x50, 0x80);
//    OV7670_write_register(0x51, 0x00);
//    OV7670_write_register(0x52, 0x22);
//    OV7670_write_register(0x53, 0x5e);
//    OV7670_write_register(0x54, 0x80);
//    OV7670_write_register(0x56, 0x40);
//    OV7670_write_register(0x58, 0x9e);
//    OV7670_write_register(0x59, 0x88);
//    OV7670_write_register(0x5a, 0x88);
//    OV7670_write_register(0x5b, 0x44);
//    OV7670_write_register(0x5c, 0x67);
//    OV7670_write_register(0x5d, 0x49);
//    OV7670_write_register(0x5e, 0x0e);
//    OV7670_write_register(0x69, 0x00);
//    OV7670_write_register(0x6a, 0x40);
//    OV7670_write_register(0x6b, 0x0a);
//    OV7670_write_register(0x6c, 0x0a);
//    OV7670_write_register(0x6d, 0x55);
//    OV7670_write_register(0x6e, 0x11);
//    OV7670_write_register(0x6f, 0x9f);
//    OV7670_write_register(0xb0, 0x84);
//}

#include <Wire.h>

#define OV7670_I2C_ADDRESS 0x21 /*TODO: write this in hex (eg. 0xAB) */


int reg11;
int reg0C;
int reg40;
int reg3e;
int reg12;
int reg42;
int reg14;

///////// Main Program //////////////
void setup() {
  String output;
  Wire.begin();
  Serial.begin(9600);
  Serial.println( "setup starting" );
  
  // TODO: READ KEY REGISTERS
  read_key_registers();  //set the global registers
  
  delay(100);
  
  // TODO: WRITE KEY REGISTERS
  //there should be 8 of them
//laasyas stuff
//  Serial.println("starting write");
//  Serial.println(OV7670_write_register( 0x12, 0x80 )); 
  //delay
  delay(100);
  Serial.println(OV7670_write_register( 0x12, 0x0C ));  //c should disable test, E should enable everything
  Serial.println(OV7670_write_register( 0x11, 0xC0 )); 
  Serial.println(OV7670_write_register( 0x0C, 0x08 )); 
  Serial.println(OV7670_write_register( 0x40, 0xD0 )); //confirmed good except output seems whack
  Serial.println(OV7670_write_register( 0x42, 0x00 ));//more color test reg 42, 0 will turn off, 08 will turn on
  Serial.println(OV7670_write_register( 0x14, 0x0B ));
   
// to turn color test on:
// replace "Serial.println(OV7670_write_register( 0x12, 0x0C ));" with  Serial.println(OV7670_write_register( 0x12, 0x0E ));
// replace "Serial.println(OV7670_write_register( 0x42, 0x00 ));" with Serial.println(OV7670_write_register( 0x42, 0x08 ));


  
//Serial.println(OV7670_write_register( 0x11, 0x40 )); //Use external clock as internal clock -- reg 11  set to 80--set to default
//  Serial.println(OV7670_write_register( 0x0C, 0x08 )); //Enable scaling -- reg 0c
//  Serial.println(OV7670_write_register( 0x40, 0xD0 )); //reg 40 Change resolution
//  Serial.println(OV7670_write_register( 0x12, 0x06 )); //change resolution
//  Serial.println(OV7670_write_register( 0x12, 0x80 ));//color test reg 12 AND reset all registers by enabling last bit
//  Serial.println(OV7670_write_register( 0x42, 0x08 ));//more color test reg 42

  //katarinas stuff
//  Serial.println(OV7670_write_register( 0x11, 0x80 )); //Use external clock as internal clock -- reg 11  set to 80--set to default
//  Serial.println(OV7670_write_register( 0x0C, 0x08 )); //Enable scaling -- reg 0c
//  Serial.println(OV7670_write_register( 0x40, 0x20 )); //reg 40 Change resolution
//  Serial.println(OV7670_write_register( 0x12, 0x8E )); //or try 8e
//  //Serial.println(OV7670_write_register( 0x12, 0x0C )); //change resolution
//  //Serial.println(OV7670_write_register( 0x12, 0x80 ));//color test reg 12 AND reset all registers by enabling last bit
//  //Serial.println(OV7670_write_register( 0x12, 0x02 ));
//  Serial.println(OV7670_write_register( 0x42, 0x08 ));//more color test reg 42

//liams stuff
//  output = OV7670_write_register( 17, reg11 | 0000010 ); //Use external clock as internal clock -- reg 11
//  Serial.println( output );
//  output = OV7670_write_register( 12, 00011100 ); //Enable scaling -- reg 0c
//  Serial.println( output );
//  output = OV7670_write_register( 64, reg40 | 00000100 ); //reg 40 Change resolution
//  Serial.println( output );
//  output = OV7670_write_register( 62, reg3e | 00000100 ); //reg 3e also changing resolution
//  Serial.println( output );
//  output = OV7670_write_register( 18, reg12 | 01000001 );//color test reg 12 AND reset all registers by enabling last bit
//  Serial.println( output );
//  output = OV7670_write_register( 66, reg42 | 00010000 );//more color test reg 42
//  Serial.println( output );
  read_key_registers();
  set_color_matrix();
    // put your setup code here, to run once:
  pinMode(8, INPUT); 
  pinMode(9, INPUT); 
  //00 means not red or blue
  //01 means red
  //10 means blue
  Serial.println( "setup finished." );
}

int MSB = 0; //leftmost
int LSB = 0; //rightmost


void loop(){
    // put your main code here, to run repeatedly:
  
  MSB = digitalRead(8);
  LSB = digitalRead(9);
  if(MSB == LOW && LSB == LOW){ //nothing
    Serial.println("no color");
  }
  else if(MSB == LOW && LSB == HIGH){
    Serial.println("RED");
  }
  else if(MSB == HIGH && LSB == LOW){
    Serial.println("BLUE");
  }
  Serial.println( "loop" );
}

///////// Function Definition //////////////
void read_key_registers(){
  /*TODO: DEFINE THIS FUNCTION*/
  reg11 = read_register_value(0x11); //external clock
  Serial.println( "reg11" );
  Serial.println( reg11 );
  reg12 = read_register_value(0x12); //enable scaling
  Serial.println( "reg12" );
  Serial.println( reg12 );
  reg40 = read_register_value(0x40); //change res 
  Serial.println( "reg40" );
  Serial.println( reg40 );
//  reg3e = read_register_value(62); //change res
//  Serial.println( reg3e );
//  reg12 = read_register_value(18); //change res
//  Serial.println( reg12 );
  reg42 = read_register_value(0x42); //change res
  Serial.println( "reg42" );
  Serial.println( reg42 );
  reg0C = read_register_value(0x0C); //change res
  Serial.println( "reg0c" );
  Serial.println( reg0C );
  reg14 = read_register_value(0x14); //change res
  Serial.println( "reg14" );
  Serial.println( reg14 );
  Serial.println( "finished read" );
  
}


byte read_register_value(int register_address){
  byte data = 0;
  Wire.beginTransmission(OV7670_I2C_ADDRESS);
  Serial.println( "begining read_reg_val" );
  //Serial.println( register_address );
  Wire.write(register_address);
  Wire.endTransmission();
  Wire.requestFrom(OV7670_I2C_ADDRESS,1);
  while(Wire.available()<1);
  data = Wire.read();
  return data;
}


String OV7670_write(int start, const byte *pData, int size){
    int n,error;
    Wire.beginTransmission(OV7670_I2C_ADDRESS);
    Serial.println( "starting transmission" );
    n = Wire.write(start);
    if(n != 1){
      return "I2C ERROR WRITING START ADDRESS";   
    }
    n = Wire.write(pData, size);
    if(n != size){
      return "I2C ERROR WRITING DATA";
    }
    error = Wire.endTransmission(true);
    if(error != 0){
      return String(error);
    }
    return "no errors :)";
 }
 

String OV7670_write_register(int reg_address, byte data){
  return OV7670_write(reg_address, &data, 1);
 }
 

void set_color_matrix(){
    OV7670_write_register(0x4f, 0x80);
    OV7670_write_register(0x50, 0x80);
    OV7670_write_register(0x51, 0x00);
    OV7670_write_register(0x52, 0x22);
    OV7670_write_register(0x53, 0x5e);
    OV7670_write_register(0x54, 0x80);
    OV7670_write_register(0x56, 0x40);
    OV7670_write_register(0x58, 0x9e);
    OV7670_write_register(0x59, 0x88);
    OV7670_write_register(0x5a, 0x88);
    OV7670_write_register(0x5b, 0x44);
    OV7670_write_register(0x5c, 0x67);
    OV7670_write_register(0x5d, 0x49);
    OV7670_write_register(0x5e, 0x0e);
    OV7670_write_register(0x69, 0x00);
    OV7670_write_register(0x6a, 0x40);
    OV7670_write_register(0x6b, 0x0a);
    OV7670_write_register(0x6c, 0x0a);
    OV7670_write_register(0x6d, 0x55);
    OV7670_write_register(0x6e, 0x11);
    OV7670_write_register(0x6f, 0x9f);
    OV7670_write_register(0xb0, 0x84);
}
