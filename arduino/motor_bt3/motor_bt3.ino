#include <SoftwareSerial.h>

SoftwareSerial mySerial(3, 2); // RX, TX

String lPwmStr = "";
String rPwmStr = "";

int lPwm;
int rPwm;

String command = ""; // Stores response of the HC-06 Bluetooth device
String cmdStr = "";

int ledPin = 13;  // use the built in LED on pin 13 of the Uno
int state = 0;

// Motor 1
int dir1PinA = 4;
int dir2PinA = 5;
int speedPinA = 6; // Needs to be a PWM pin to be able to control motor speed

// Motor 2
int dir1PinB = 7;
int dir2PinB = 8;
int speedPinB = 9; // Needs to be a PWM pin to be able to control motor speed

char inData[10]; // Allocate some space for the string
char inChar=-1; // Where to store the character read
byte index = 0; // Index into array; where to store the character
int cmdCode = 0;
boolean readSuccess = false;
boolean clearBuf = false;

char lDir = 0;
char rDir = 0;
char lDirOld = 0;
char rDirOld = 0;

void setup() {
  // The HC-06 defaults to 9600 according to the datasheet.
  mySerial.begin(9600);
  
  pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, LOW);
    
  pinMode(dir1PinA,OUTPUT);
  pinMode(dir2PinA,OUTPUT);
  pinMode(speedPinA,OUTPUT);
  pinMode(dir1PinB,OUTPUT);
  pinMode(dir2PinB,OUTPUT);
  pinMode(speedPinB,OUTPUT);
    
}

void loop() {
    
  readCommand();
  
  if (readSuccess) {
    parseCommand();
    readSuccess = false;
    updateMotor(dir1PinA, dir2PinA, speedPinA, &lDirOld, lDir, lPwm);
    updateMotor(dir1PinB, dir2PinB, speedPinB, &rDirOld, rDir, rPwm);
  }
  if (clearBuf) {
    doClearBuf();
  }
    
}

void updateMotor(int dir1Pin, int dir2Pin, int speedPin, char * dirOldValue, char dirValue, int pwmVal) {
  if (*dirOldValue != dirValue) {
    *dirOldValue = dirValue;
    switch (dirValue) {
      case 'f':    
        digitalWrite(dir1Pin, LOW);
        digitalWrite(dir2Pin, HIGH);
        break;
      case 'b':    
        digitalWrite(dir1Pin, HIGH);
        digitalWrite(dir2Pin, LOW);
        break;
   } 
  }
   analogWrite(speedPin, pwmVal);//Sets speed variable via PWM 
}

void readCommand() {
    while (mySerial.available() > 0) {
       inChar = mySerial.read(); // Read a character
       if (inChar == 'z') {
         inData[index] = '\0'; // Null terminate the string
         readSuccess = true;
         clearBuf = true;
         cmdCode = 5;
         break;
       }

       inData[index] = inChar; // Store it
       index++; // Increment where to write next
      if (index >= 9) {
        clearBuf = true;
        break;
      }
    }
}

void doClearBuf() {
        for (int i=0;i<9;i++) {
            inData[i]=0;
        }
        index=0;
        clearBuf = false;
}

void parseCommand() {
      cmdStr = String(inData);
      lDir = cmdStr.charAt(0);
      rDir = cmdStr.charAt(4);
      lPwmStr = cmdStr.substring(1, 4);
      rPwmStr = cmdStr.substring(5, 8);
      lPwm = lPwmStr.toInt();
      rPwm = rPwmStr.toInt();
}

