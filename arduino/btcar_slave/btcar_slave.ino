/*
 */
#include <Servo.h>
//#include <NewPing.h>

const int hServoPin = 9; // Servo library disables analogWrite() (PWM) functionality on pins 9 and 10 
const int vServoPin = 10; // Servo library disables analogWrite() (PWM) functionality on pins 9 and 10
Servo servoH;          // horizontal servo
Servo servoV;         // vertical servo
int hServoVal;
int vServoVal;

//#define TEST
//#define DEBUG

#define TRIGGER_PIN  4  // Arduino pin tied to trigger pin on ping sensor.
#define ECHO_PIN     13  // Arduino pin tied to echo pin on ping sensor.
#define MAX_DISTANCE 200 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

#define MAX_PACKET_SIZE 20
#define PACKET_TIMEOUT 100
char cmdBuf[MAX_PACKET_SIZE + 1];
unsigned long packetTimeStart = 0;
unsigned long packetTimeCurrent = 0;
boolean startPacketReading = false;
int packetBufCounter = 0;

//NewPing sonar(A2, A3, MAX_DISTANCE);
//NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

unsigned int pingSpeed = 50; // How frequently are we going to send out a ping (in milliseconds). 50ms would be 20 times a second.
unsigned long pingTimer;     // Holds the next ping time.
float pingDistance = 999.0;
float pingDistance2 = 999.0;
boolean isObstacleForward = false;
unsigned long pingTime1 = 0;
unsigned long pingTime2 = 0;

char cmd = 0;
char dirCmd = 0;
byte cmds[4];

boolean cmdUpdateMotor = false;
boolean cmdStopBeforeObstacle = false;
boolean cmdUpdateServoH = false;
boolean cmdUpdateServoV = false;
char lDir = 0;
char rDir = 0;
int lPwm = 0;
int rPwm = 0;
boolean stopBeforeObstacle = false;
boolean stoppedBeforeObstacle = false;

// motors start

#define FORWARD 1
#define BACKWARD 2
#define BRAKE 3
#define RELEASE 4

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

// ampers start
#define AMP_OFFSET 2500
#define BATT_AMP_VOLTS_PER_AMP 66
#define BATT_AMP_PIN A3
#define BATT_AMP_READ_SPEED 50
unsigned long readBattAmpTimer;
float batteryAmps = 0.0;
// ampers end

// voltage start
#define BATT_VOLT_PIN A5
#define BATT_VOLT_READ_SPEED 50
unsigned long readBattVoltTimer;
float batteryVoltage = 0.0;
#define BATT_VOLT_R1 51000 
#define BATT_VOLT_R2 47000 
const float BATT_VOLT_VD = (BATT_VOLT_R1 + BATT_VOLT_R2)/BATT_VOLT_R2;
// voltage end

// info start
#define INFO_SPEED 50
unsigned long infoTimer;
String infoStr;
static char outstr[15];
// info end

void setup() {
  Serial.begin(38400);

  cmdBuf[MAX_PACKET_SIZE] = 0; //null terminated string

  pinMode(BATT_AMP_PIN, INPUT);
  pinMode(BATT_VOLT_PIN, INPUT);

  pingTimer = readBattVoltTimer = readBattAmpTimer = infoTimer = millis(); // Start now.
  
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
#ifdef TEST
  if (true) {
    testMotorShield();
    return;
  }
#endif

  // Notice how there's no delays in this sketch to allow you to do other processing in-line while doing distance pings.
  if (millis() >= pingTimer) {   // pingSpeed milliseconds since last ping, do another ping.
    pingTimer += pingSpeed;      // Set the next ping time.
    //sonar.ping_timer(echoCheck); // Send out the ping, calls "echoCheck" function every 24uS where you can check the ping status.
    //sonar2.ping_timer(echoCheck);
  }

  cmdUpdateMotor = false;
  cmdStopBeforeObstacle = false;
  cmdUpdateServoH = false;
  cmdUpdateServoV = false;

  if (stopBeforeObstacle) {
    if (isObstacleForward && (lDir == 'f') && (rDir == 'f')) {
      stoppedBeforeObstacle = true;
      updateMotorShield('s', 's', 0, 0);
    } 
    else {
      if (stoppedBeforeObstacle) {
        stoppedBeforeObstacle = false;
        updateMotorShield(lDir, rDir, lPwm, rPwm);
      }
    }
  }

  pingTime2 = millis();
  if (pingTime2 - pingTime1 > 500) {
    pingTime1 = pingTime2;
    //bt.print("distance: ");
    //bt.println(pingDistance);
  }
  if (Serial.available() > 0) {
    cmd = Serial.read();
    //Serial.print(cmd);
    //bt.print("I received: ");
    //bt.println(cmd);
    if (startPacketReading) {
      packetTimeCurrent = millis();
      if (packetTimeCurrent - packetTimeStart > PACKET_TIMEOUT) {
        //timeout
        cmdBuf[packetBufCounter] = 0;
#ifdef DEBUG
        Serial.print("timeout, ");
        Serial.print("buf: ");
        Serial.println(cmdBuf);
#endif

          packetBufCounter = 0;
        startPacketReading = false;
      } 
      else if (packetBufCounter >= MAX_PACKET_SIZE) {
        //no end of packet symbol
        cmdBuf[packetBufCounter] = 0;
#ifdef DEBUG
        Serial.print("no end of packet, ");
        Serial.print("buf: ");
        Serial.println(cmdBuf);
#endif

          packetBufCounter = 0;
        startPacketReading = false;
      } 
      else {
        if (cmd == 'z') {
          //end of packet
          //parse packet
          cmdBuf[packetBufCounter] = 0;
#ifdef DEBUG
          Serial.print("ok, buf: ");
          Serial.println(cmdBuf);
#endif

            boolean parsedOk = parseCmdPacket();
          if (parsedOk) {
            if (cmdUpdateMotor) {
              updateMotorShield(lDir, rDir, lPwm, rPwm);
            }
            if (cmdUpdateServoH) {
              updateServo(servoH, hServoVal);
            }
            if (cmdUpdateServoV) {
              updateServo(servoV, vServoVal);
            }
          }

          packetBufCounter = 0;
          startPacketReading = false;
        } 
        else {
          cmdBuf[packetBufCounter] = cmd;
          packetBufCounter++;
        }
      }
    }
    if (cmd == 'a') {
      packetTimeStart = millis();
      startPacketReading = true;
      for (int i = 0; i < MAX_PACKET_SIZE; i++) {
        cmdBuf[i] = 0;
      }
    }

    // say what you got:
    //Serial.print("I received: ");
    //Serial.println(cmd, DEC);

  }
  
  if (millis() >= readBattAmpTimer) {
    readBattAmpTimer += BATT_AMP_READ_SPEED;
    readBatteryAmps();
  }

  if (millis() >= readBattVoltTimer) {
    readBattVoltTimer += BATT_VOLT_READ_SPEED;
    readBatteryVoltage();
  }

  if (millis() >= infoTimer) {
    infoTimer += INFO_SPEED;
    showInfo();
  }

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

void echoCheck() { // Timer2 interrupt calls this function every 24uS where you can check the ping status.
  // Don't do anything here!
  float p1 = -1;
  float p2 = -1;
//  if (sonar.check_timer()) { // This is how you check to see if the ping was received.
    // Here's where you can add code.
//    pingDistance = sonar.ping_result / US_ROUNDTRIP_CM;
//   isObstacleForward = pingDistance < 30;
//    p1 = pingDistance;
//   }
//  if (sonar2.check_timer()) {
//    pingDistance2 = sonar2.ping_result / US_ROUNDTRIP_CM;
//    p2 = pingDistance2;
//  }
  if (p1 > 0 || p2 > 0) {
    //Serial.print(p1);
    //Serial.print("   ");
    //Serial.println(p2);
  }
  // Don't do anything here!
}

boolean parseCmdPacket() {
  String cmdStr = String(cmdBuf);
  int strLen = cmdStr.length();
  if (strLen == 0) {
    return false;
  }

  int pos = 0;
  boolean err = false;
  char actionCommand;
  while (pos <= strLen - 1) {
    actionCommand = cmdStr.charAt(pos);
    int cmdLength = 0;
    if (actionCommand == 'm') {
      cmdLength = 8;
      if (pos + cmdLength <= strLen - 1) {
        cmdUpdateMotor = true;
        parseMotorCommand(cmdStr, pos);
      } 
      else {
        err = true;
        break;
      }
    }
    else if (actionCommand == 'o')  {
      cmdLength = 1;
      cmdStopBeforeObstacle = true;
      parseStopBeforeObstacle(cmdStr, pos);
    }
    else if (actionCommand == 'h') {
      cmdLength = 3;
      if (pos + cmdLength <= strLen - 1) {
        cmdUpdateServoH = true;
        parseServoCommandH(cmdStr, pos);
      } 
      else {
        err = true;
        break;
      }
    }
    else if (actionCommand == 'v') {
      cmdLength = 3;
      if (pos + cmdLength <= strLen - 1) {
        cmdUpdateServoV = true;
        parseServoCommandV(cmdStr, pos);
      } 
      else {
        err = true;
        break;
      }
    }
    else {
      err = true;
      break;
    }
    pos = pos + 1 + cmdLength;
  }

  return !err;
}

void parseServoCommandH(String cmdStr, int pos) {
  String str = cmdStr.substring(pos + 1, pos + 4);
  hServoVal = str.toInt();
}

void parseServoCommandV(String cmdStr, int pos) {
  String str = cmdStr.substring(pos + 1, pos + 4);
  vServoVal = str.toInt();
}

void parseMotorCommand(String cmdStr, int pos) {
  lDir = cmdStr.charAt(pos + 1);
  rDir = cmdStr.charAt(pos + 5);
  String lPwmStr = cmdStr.substring(pos + 2, pos + 5);
  String rPwmStr = cmdStr.substring(pos + 6, pos + 9);
  lPwm = lPwmStr.toInt();
  rPwm = rPwmStr.toInt();
  //Serial.println(lPwm);
  //      Serial.println(rPwm);
}

void parseStopBeforeObstacle(String cmdStr, int pos) {
  stopBeforeObstacle = cmdStr.charAt(pos + 1) == '1';
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

void readBatteryVoltage() {
  batteryVoltage = (analogRead(BATT_VOLT_PIN) * 5.0 * BATT_VOLT_VD) / 1024.0;
}

void readBatteryAmps() {
  //TODO 1023 ???
  batteryAmps = ((((analogRead(BATT_AMP_PIN) / 1023.0) * 5000) - AMP_OFFSET) / BATT_AMP_VOLTS_PER_AMP);
}

void showInfo() {
     //infoStr = "";
     //dtostrf(batteryAmps,7, 3, outstr);
     //infoStr += "amps: ";
     //infoStr += outstr;

     infoStr = "";
     infoStr += "uptime: ";
     infoStr += millis();
     
     dtostrf(batteryVoltage,7, 3, outstr);
     infoStr += ", volts: ";
     infoStr += outstr;
     
     dtostrf(pingDistance,7, 3, outstr);
     infoStr += ", ping: ";
     infoStr += outstr;
     
     Serial.println(infoStr);
}

#ifdef TEST
void testMotorShield() {
  delay(1000);
  
  char f = 'f';
  char b = 'b';
  char s = 's';
  int sp = 100;
  int maxSpeed = 255;
  int del1 = 3000;
  int del2 = 1000;

  updateMotorShield(f, s, sp, 0);
  delay(del1);
  updateMotorShield(s, s, 0, 0);
  delay(del2);

  updateMotorShield(b, s, sp, 0);
  delay(del1);
  updateMotorShield(s, s, 0, 0);
  delay(del2);

  updateMotorShield(s, f, 0, sp);
  delay(del1);
  updateMotorShield(s, s, 0, 0);
  delay(del2);

  updateMotorShield(s, b, 0, sp);
  delay(del1);
  updateMotorShield(s, s, 0, 0);
  delay(del2);

  updateMotorShield(f, f, sp, sp);
  delay(del1);
  updateMotorShield(s, s, 0, 0);
  delay(del2);

  updateMotorShield(b, b, sp, sp);
  delay(del1);
  updateMotorShield(s, s, 0, 0);
  delay(del2);

  updateMotorShield(f, b, sp, sp);
  delay(del1);
  updateMotorShield(s, s, 0, 0);
  delay(del2);

  updateMotorShield(b, f, sp, sp);
  delay(del1);
  updateMotorShield(s, s, 0, 0);
  delay(del2);

  updateMotorShield(f, f, maxSpeed, maxSpeed);
  delay(del1);
  updateMotorShield(s, s, 0, 0);
  delay(del2);

  delay(5000);
}
#endif




