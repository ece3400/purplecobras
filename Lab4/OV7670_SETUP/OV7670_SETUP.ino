#include <Wire.h>

#define OV7670_I2C_ADDRESS 0x21 /*TODO: write this in hex (eg. 0xAB) */


///////// Main Program //////////////
void setup() {
  Wire.begin();
  Serial.begin(9600);
  // TODO: READ KEY REGISTERS
  int resetRegInit = read_register_value(0x12);
  int scalingRegInit1 = read_register_value(0x0C);
  int scalingRegInit2 = read_register_value(0x3E);
  int clkRegInit = read_register_value(0x11);
  int testRegInit1 = read_register_value(0x70);
  int testRegInit2 = read_register_value(0x71);
  int formatRegInit = read_register_value(0x40);
  
  delay(100);

  Serial.println("Desired Values:");
  Serial.println(resetRegInit & 0b01111111);
  Serial.println(scalingRegInit1 | 0b00001000);
  Serial.println(scalingRegInit2 | 0b00001000);
  Serial.println((formatRegInit | 0b00010000) & 0b11101111);
  Serial.println(clkRegInit | 0b01000000);
  Serial.println((clkRegInit | 0b10000000) & 0b01111111);
  Serial.println("End Desired Values");
  
  // TODO: WRITE KEY REGISTERS
  OV7670_write_register(0x12, resetRegInit | 0b10000000); //reset registers
  resetRegInit = read_register_value(0x12);
  scalingRegInit1 = read_register_value(0x0C);
  scalingRegInit2 = read_register_value(0x3E);
  clkRegInit = read_register_value(0x11);
  testRegInit1 = read_register_value(0x70);
  testRegInit2 = read_register_value(0x71);
  formatRegInit = read_register_value(0x40);
  Serial.println("Reset Values:");
  Serial.println(resetRegInit & 0b01111111);
  Serial.println(scalingRegInit1 | 0b00001000);
  Serial.println(scalingRegInit2 | 0b00001000);
  Serial.println((formatRegInit | 0b00010000) & 0b11101111);
  Serial.println(clkRegInit | 0b01000000);
  Serial.println((clkRegInit | 0b10000000) & 0b01111111);
  Serial.println("End Reset Values");
  
  OV7670_write_register(0x12, resetRegInit & 0b01111111); //registers back to normal mode
  OV7670_write_register(0x0C, scalingRegInit1 | 0b00001000); //enable scaling
  OV7670_write_register(0x3E, scalingRegInit2 | 0b00001000); //enable scaling with custom resolution
  OV7670_write_register(0x40, (formatRegInit | 0b00010000) & 0b11101111); //enable scaling with custom resolution
  OV7670_write_register(0x11, clkRegInit | 0b01000000); //use external clock
  OV7670_write_register(0x70, clkRegInit | 0b10000000); //color bar test
  OV7670_write_register(0x70, clkRegInit & 0b01111111); //color bar test


  set_color_matrix();

  
  read_key_registers();
}

void loop(){
  //read_key_registers();
  delay(1000);
}


///////// Function Definition //////////////
void read_key_registers(){
  /*TODO: DEFINE THIS FUNCTION*/
  Serial.println(read_register_value(0x12));
  Serial.println(read_register_value(0x0C));
  Serial.println(read_register_value(0x3E));
  Serial.println(read_register_value(0x40));
  Serial.println(read_register_value(0x11));
  Serial.println(read_register_value(0x70));
}

byte read_register_value(int register_address){
  byte data = 0;
  Wire.beginTransmission(OV7670_I2C_ADDRESS);
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
