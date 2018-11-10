int sensorL = A1;
int sensorR = A2;
int lineVoltage = 520;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

int readL[3] = {lineVoltage + 100, lineVoltage + 100, lineVoltage + 100};
int readR[3] = {lineVoltage + 100, lineVoltage + 100, lineVoltage + 100};

void loop() {
  // put your main code here, to run repeatedly:

  //LINE SENSORS
  readR[0] = readR[1];
  readR[1] = readR[2];
  readR[2] = analogRead(sensorR);
  readL[0] = readL[1];
  readL[1] = readL[2];
  readL[2] = analogRead(sensorL);
  
  double leftAverage = (readL[0] + readL[1] + readL[2])/3;
  double rightAverage = (readR[0] + readR[1] + readR[2])/3;
 
  Serial.print("left : ");
  Serial.println(leftAverage);
  Serial.print("right : ");
  Serial.println(rightAverage);
}
