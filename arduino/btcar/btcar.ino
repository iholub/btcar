/*
 */
#include <SoftwareSerial.h>
#include <NewPing.h>

//#define TEST
//#define DEBUG

#define BLUETOOTH_RX A6
#define BLUETOOTH_TX A7

#define TRIGGER_PIN  4  // Arduino pin tied to trigger pin on ping sensor.
#define ECHO_PIN     3  // Arduino pin tied to echo pin on ping sensor.
#define MAX_DISTANCE 200 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

#define MAX_PACKET_SIZE 20
#define PACKET_TIMEOUT 100
char cmdBuf[MAX_PACKET_SIZE + 1];
unsigned long packetTimeStart = 0;
unsigned long packetTimeCurrent = 0;
boolean startPacketReading = false;
int packetBufCounter = 0;

//NewPing sonar(13, 7, MAX_DISTANCE);
//NewPing sonar(A2, A3, MAX_DISTANCE);
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.
//NewPing sonar2(13, 7, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

unsigned int pingSpeed = 50; // How frequently are we going to send out a ping (in milliseconds). 50ms would be 20 times a second.
unsigned long pingTimer;     // Holds the next ping time.
float pingDistance = 999.0;
float pingDistance2 = 999.0;
boolean isObstacleForward = false;
unsigned long pingTime1 = 0;
unsigned long pingTime2 = 0;


SoftwareSerial bt(BLUETOOTH_RX, BLUETOOTH_TX); // RX, TX

char cmd = 0;
char dirCmd = 0;
byte buf[8];
byte cmds[4];
byte sp = 255;

boolean cmdUpdateMotor = false;
boolean cmdStopBeforeObstacle = false;
char lDir = 0;
char rDir = 0;
int lPwm = 0;
int rPwm = 0;
boolean stopBeforeObstacle = false;
boolean stoppedBeforeObstacle = false;

void setup() {
  Serial.begin(38400);
#ifdef DEBUG 
  Serial.begin(9600);  
#endif
  bt.begin(38400);
  bt.listen();  

  cmdBuf[MAX_PACKET_SIZE] = 0; //null terminated string

  pingTimer = millis(); // Start now.
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
    sonar.ping_timer(echoCheck); // Send out the ping, calls "echoCheck" function every 24uS where you can check the ping status.
    //sonar2.ping_timer(echoCheck);
  }

  cmdUpdateMotor = false;
  cmdStopBeforeObstacle = false;

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
  if (bt.available() > 0) {
    cmd = bt.read();
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
        if (cmd == '$') {
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
    if (cmd == '#') {
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
}

byte buildDir(char dirValue) {
  byte res = 0;
  switch (dirValue) {
  case 'f':    
    res = 98;
    break;
  case 'b':
    res = 99;
    break;
  case 's':
    res = 100;
    break;
  }
  return res;
}

void updateMotorShield(char lDir, char rDir, int lPwm, int rPwm) {
  //TODO 
  buf[0] = buf[1] = buildDir(lDir);
  buf[2] = buf[3] = buildDir(rDir);
  buf[4] = buf[5] = lPwm;
  buf[6] = buf[7] = rPwm;
  //mt.write(buf, 8);
  //mt.flush();
#ifdef DEBUG
  Serial.print("shield: ");
  for (int i = 0; i < 8; i++) {
    Serial.print(buf[i]);
    Serial.print(" ");
  }
  Serial.println();
#endif
}

void echoCheck() { // Timer2 interrupt calls this function every 24uS where you can check the ping status.
  // Don't do anything here!
  float p1 = -1;
  float p2 = -1;
  if (sonar.check_timer()) { // This is how you check to see if the ping was received.
    // Here's where you can add code.
    pingDistance = sonar.ping_result / US_ROUNDTRIP_CM;
    isObstacleForward = pingDistance < 30;
    p1 = pingDistance;
   }
//  if (sonar2.check_timer()) {
//    pingDistance2 = sonar2.ping_result / US_ROUNDTRIP_CM;
//    p2 = pingDistance2;
//  }
  if (p1 > 0 || p2 > 0) {
    Serial.print(p1);
    Serial.print("   ");
    Serial.println(p2);
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
    else {
      err = true;
      break;
    }
    pos = pos + 1 + cmdLength;
  }

  return !err;
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

#ifdef TEST
void testMotorShield() {
  char f = 'f';
  char b = 'b';
  char s = 's';
  int speed = 100;
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




