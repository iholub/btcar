/*
 */
#include <Servo.h>
#include <Wire.h>

#define SLAVE_ADDR 0x31 // Slave address, should be changed for other slaves
byte receivedCommands[20];

const int hServoPin = 9; // Servo library disables analogWrite() (PWM) functionality on pins 9 and 10 
const int vServoPin = 10; // Servo library disables analogWrite() (PWM) functionality on pins 9 and 10
Servo servoH;          // horizontal servo
Servo servoV;         // vertical servo
int hServoVal;
int vServoVal;

boolean cmdUpdateMotor = false;
boolean cmdUpdateServoH = false;
boolean cmdUpdateServoV = false;
char lDir = 0;
char rDir = 0;
int lPwm = 0;
int rPwm = 0;

// motors start

//Pin connected to ST_CP of 74HC595
int latchPin = 8;
//Pin connected to SH_CP of 74HC595
int clockPin = 12;
////Pin connected to DS of 74HC595
int dataPin = 7;

int pwmPins[4]  = {5, 6, 3, 11};

int ind;
int pwmSpeed = 0;

uint8_t latch_state = 0;

int dir1Pins[4] = {0, 2, 5, 7};
int dir2Pins[4] = {1, 3, 4, 6};
char dirs[4] = {'s', 's', 's', 's'};

// motors end

void setup() {
  Wire.begin(SLAVE_ADDR);
  Wire.onReceive(receiveEvent);

  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);  
  pinMode(clockPin, OUTPUT);
  
  servoH.attach(hServoPin);
  servoV.attach(vServoPin);

  for (ind = 0; ind < 4; ind++) {
    pinMode(pwmPins[ind], OUTPUT);
  }
 
}

void loop() {
}

void updateServo(Servo servo, int value) {
  servo.write(value);
}

void updateMotorShield(char lDir, char rDir, int lPwm, int rPwm) {
  dirs[0] = dirs[1] = lDir;
  dirs[2] = dirs[3] = rDir;
  for (int i = 0; i < 4; i++) {
    updateShiftRegister(dir1Pins[i], dir2Pins[i], dirs[i]);
  }
  
 digitalWrite(latchPin, LOW);
 shiftOut(dataPin, clockPin, MSBFIRST, latch_state);
 digitalWrite(latchPin, HIGH);
 
 analogWrite(pwmPins[0], lPwm);
 analogWrite(pwmPins[1], lPwm);
 analogWrite(pwmPins[2], rPwm);
 analogWrite(pwmPins[3], rPwm);
}

void updateShiftRegister(uint8_t a, uint8_t b, char cmd) {
  switch (cmd) {
  case 'f':
    latch_state |= _BV(a);
    latch_state &= ~_BV(b); 
    break;
  case 'b':
    latch_state &= ~_BV(a);
    latch_state |= _BV(b); 
    break;
  case 's':
    latch_state &= ~_BV(a);     // A and B both low
    latch_state &= ~_BV(b); 
    break;
  }
}

void receiveEvent(int bytesReceived)
{
  for (int a = 0; a < bytesReceived; a++)
  {
    receivedCommands[a] = Wire.read();
  }

  cmdUpdateServoH = false;
  cmdUpdateServoV = false;

  int ind = 0;
  while (ind <= bytesReceived - 1) {
    switch(receivedCommands[ind]){
      case 0x0A:
        hServoVal = receivedCommands[ind + 1];
        cmdUpdateServoH = true;
        ind += 2;
        break;
      case 0x0B:
        vServoVal = receivedCommands[ind + 1];
        cmdUpdateServoV = true;
        ind += 2;
        break;
      default:
        return; // ignore the commands and return
     }    
  }
  
  if (cmdUpdateServoH) {
    updateServo(servoH, hServoVal);
  }
  if (cmdUpdateServoV) {
    updateServo(servoV, vServoVal);
  }
  
  }


