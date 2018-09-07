#include <Servo.h>

Servo myservo;

void setup() {
  // put your setup code here, to run once:
  pinMode(A5, INPUT);
  Serial.begin(9600);
  pinMode(9, OUTPUT);
  myservo.attach(5);
  myservo.write(90);
}

int x;
void loop() {
  x = analogRead(A5);
  int abc = map(x, 0, 1023, 0, 180);
  Serial.println(x);
  analogWrite(9, x);
  myservo.write(abc);
  delay(500);
}
