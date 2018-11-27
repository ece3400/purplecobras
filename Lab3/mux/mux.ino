//wall selector
int s2 = 12;
int s1 = 11;
int s0 = 10;
//wall sensor
int walls = A3;
void setup() {
 Serial.begin(9600);
// pinMode(s0, OUTPUT);
// pinMode(s1, OUTPUT);
// pinMode(s2, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
//  digitalWrite(s2, LOW);
//  digitalWrite(s1, LOW);
//  digitalWrite(s0, HIGH);

  int wall = analogRead(A3);
  
  Serial.println(wall);

}

//bool detectLeftWall() {
//  digitalWrite(s2, LOW);
//  digitalWrite(s1, LOW);
//  digitalWrite(s0, LOW);
//
//  read_wallL = analogRead(walls);
//
//  if (read_wallL >= LRwalls) {
//    return true;
//  }
//  else {
//    return false;
//  }
//}
//
//bool detectFrontWall() {
//  digitalWrite(s2, LOW);
//  digitalWrite(s1, LOW);
//  digitalWrite(s0, LOW);
//
//  read_wallF = analogRead(walls);
//
//  if (read_wallF >= Fwall) {
//    return true;
//  }
//  else {
//    return false;
//  }
//}
//
//bool detectRightWall() {
//  digitalWrite(s2, LOW);
//  digitalWrite(s1, LOW);
//  digitalWrite(s0, LOW);
//
//  read_wallR = analogRead(walls);
//
//  if (read_wallL >= LRwalls) {
//    return true;
//  }
//  else {
//    return false;
//  }
//}
