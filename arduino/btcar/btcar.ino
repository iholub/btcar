/*
 */
#include <SoftwareSerial.h>

SoftwareSerial bt(10, 11); // RX, TX

const int m1Dir1Pin = 4;
const int m1Dir2Pin = 5;
const int m1PwmPin = 3;

const int m2Dir1Pin = 7;
const int m2Dir2Pin = 8;
const int m2PwmPin = 6;

char cmd = 0;
int speedVal = 255;

void setup() {   
  Serial.begin(9600);  
  bt.begin(38400);

  pinMode(m1Dir1Pin,OUTPUT);
  pinMode(m1Dir2Pin,OUTPUT);
  pinMode(m1PwmPin,OUTPUT);

  pinMode(m2Dir1Pin,OUTPUT);
  pinMode(m2Dir2Pin,OUTPUT);
  pinMode(m2PwmPin,OUTPUT);

  updateSpeedAll();
}

void loop() {

  //  test();
  //   if (true) {
  //    return;
  //  }
  //  if (Serial.available()) {
  //    char ccc = Serial.read();
  //     Serial.write(ccc);
  //     Serial.write(ccc + 1);
  //     return;
  //  }

  if (bt.available() > 0) {
    cmd = bt.read();
    bt.print("I received: ");
    bt.println(cmd);

    // say what you got:
    //Serial.print("I received: ");
    //Serial.println(incomingByte, DEC);
    switch (cmd) {
    case 'w'://forward
      updateMotor(m1Dir1Pin, m1Dir2Pin, m1PwmPin, 1);
      updateMotor(m2Dir1Pin, m2Dir2Pin, m2PwmPin, 1);
      break;
    case 'a'://rotate left
      updateMotor(m1Dir1Pin, m1Dir2Pin, m1PwmPin, 2);
      updateMotor(m2Dir1Pin, m2Dir2Pin, m2PwmPin, 1);
      break;
    case 's'://stop
      updateMotor(m1Dir1Pin, m1Dir2Pin, m1PwmPin, 0);
      updateMotor(m2Dir1Pin, m2Dir2Pin, m2PwmPin, 0);
      break;
    case 'd'://rotate right
      updateMotor(m1Dir1Pin, m1Dir2Pin, m1PwmPin, 1);
      updateMotor(m2Dir1Pin, m2Dir2Pin, m2PwmPin, 2);
      break;
    case 'z'://backwards
      updateMotor(m1Dir1Pin, m1Dir2Pin, m1PwmPin, 2);
      updateMotor(m2Dir1Pin, m2Dir2Pin, m2PwmPin, 2);
      break;
    case 'p'://breakes
      updateMotor(m1Dir1Pin, m1Dir2Pin, m1PwmPin, 3);
      updateMotor(m2Dir1Pin, m2Dir2Pin, m2PwmPin, 3);
      break;
    case 'u'://speed max
      speedVal = 255;
      updateSpeedAll();
      break;
    case 'h'://speed middle
      speedVal = 175;
      updateSpeedAll();
      break;
    case 'b'://speed low
      speedVal = 100;
      updateSpeedAll();
      break;
    }
  }
}

void updateMotor(int dir1Pin, int dir2Pin, int pwmPin, int cmd) {
  switch (cmd) {
  case 0:
    digitalWrite(dir1Pin, LOW);
    digitalWrite(dir2Pin, LOW);
    break;
  case 1:
    digitalWrite(dir1Pin, LOW);
    digitalWrite(dir2Pin, HIGH);
    break;
  case 2:
    digitalWrite(dir1Pin, HIGH);
    digitalWrite(dir2Pin, LOW);
    break;
  case 3:
    digitalWrite(dir1Pin, HIGH);
    digitalWrite(dir2Pin, HIGH);
    break;
  }
}

void updateSpeedAll() {
  analogWrite(m1PwmPin, speedVal);
  analogWrite(m2PwmPin, speedVal);
}


