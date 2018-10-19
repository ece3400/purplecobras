#define LOG_OUT 1 // use the log output function
#define FFT_N 128 // set to 128 point fft

#include <FFT.h> // include the library

//analogRead code from Team Alpha

void setup() {
  Serial.begin(115200); // use the serial port
}

//graph from serial output to find the bin for 660hz
//test frequencies close to this to try to ensure we are exact as possible
//(hopefully this works)
void loop() {
  while(1) {
    cli();
    for (int i = 0 ; i < 256 ; i += 2) {
      fft_input[i] = analogRead(A4); // <-- NOTE THIS LINE
      fft_input[i+1] = 0;
    }
    fft_window();
    fft_reorder();
    fft_run();
    fft_mag_log();
    sei();
    Serial.println("start");
    for (byte i = 0 ; i < FFT_N/2 ; i++) {
      Serial.println(fft_log_out[i]);
    }
  }
}
