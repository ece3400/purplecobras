/*
fft_adc_serial.pde
guest openmusiclabs.com 7.7.14
example sketch for testing the fft library.
it takes in data on ADC0 (Analog0) and processes them
with the fft. the data is sent out over the serial
port at 115.2kb.
*/

#define LOG_OUT 1 // use the log output function
#define FFT_N 128 // set to 128 point fft

#include <FFT.h> // include the library

int default_timsk = TIMSK0;
int default_adcsra = ADCSRA;
int default_admux = ADMUX;
int default_didr = DIDR0;

int robot = 0;

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
}

void adc_Setup() {
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0
}

void loop() {
  adc_Setup();
  robot = detectRobot();
  adc_Reset();
  digitalWrite(LED_BUILTIN, robot);
  delay(5000);
}

void adc_Reset() {
  TIMSK0 = default_timsk;
  ADCSRA = default_adcsra;
  ADMUX = default_admux;
  DIDR0 = default_didr;
}

int detectRobot() {
  cli();
  for (int i = 0 ; i < 256 ; i += 2) { // save 128 samples
      while(!(ADCSRA & 0x10)); // wait for adc to be ready
      ADCSRA = 0xf5; // restart adc
      byte m = ADCL; // fetch adc data
      byte j = ADCH;
      int k = (j << 8) | m; // form into an int
      k -= 0x0200; // form into a signed int
      k <<= 6; // form into a 16b signed int
      fft_input[i] = k; // put real data into even bins
      fft_input[i+1] = 0; // set odd bins to 0
    }

  fft_window(); // window the data for better frequency response
  fft_reorder(); // reorder the data before doing the fft
  fft_run(); // process the data in the fft
  fft_mag_log(); // take the output of the fft
  sei();
  Serial.println("start");
  for (byte i = 0 ; i < FFT_N/2 ; i++) { 
      Serial.println(fft_log_out[i]); // send out the data
  }
//  if (fft_log_out[23] >= 100) {
//    Serial.println("ir signal");
//    return 1;  
//  }
//  else {
//    Serial.println("no signal");
//    return 0;
//  }
}
