/*
 */
#include <SoftwareSerial.h>
#include <NewPing.h>

#define BLUETOOTH_RX 10
#define BLUETOOTH_TX 9
#define MOTOR_SHIELD_RX 2
#define MOTOR_SHIELD_TX 3

#define TRIGGER_PIN  12  // Arduino pin tied to trigger pin on ping sensor.
#define ECHO_PIN     11  // Arduino pin tied to echo pin on ping sensor.
#define MAX_DISTANCE 200 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

#define MAX_PACKET_SIZE 20
#define PACKET_TIMEOUT 100
char cmdBuf[MAX_PACKET_SIZE + 1];
unsigned long packetTimeStart = 0;
unsigned long packetTimeCurrent = 0;
boolean startPacketReading = false;
int packetBufCounter = 0;

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

unsigned int pingSpeed = 50; // How frequently are we going to send out a ping (in milliseconds). 50ms would be 20 times a second.
unsigned long pingTimer;     // Holds the next ping time.
float pingDistance = 999.0;
boolean isObstacleForward = false;
unsigned long pingTime1 = 0;
unsigned long pingTime2 = 0;


SoftwareSerial bt(BLUETOOTH_RX, BLUETOOTH_TX); // RX, TX
SoftwareSerial mt(MOTOR_SHIELD_RX, MOTOR_SHIELD_TX); // RX, TX

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
  Serial.begin(9600);  
  bt.begin(38400);
  mt.begin(9600);
  bt.listen();  

  cmdBuf[MAX_PACKET_SIZE] = 0; //null terminated string

  pingTimer = millis(); // Start now.
}

void loop() {
  // Notice how there's no delays in this sketch to allow you to do other processing in-line while doing distance pings.
  if (millis() >= pingTimer) {   // pingSpeed milliseconds since last ping, do another ping.
    pingTimer += pingSpeed;      // Set the next ping time.
    sonar.ping_timer(echoCheck); // Send out the ping, calls "echoCheck" function every 24uS where you can check the ping status.
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
        Serial.print("timeout, ");
        cmdBuf[packetBufCounter] = 0;
        Serial.print("buf: ");
        Serial.println(cmdBuf);

        packetBufCounter = 0;
        startPacketReading = false;
      } 
      else if (packetBufCounter >= MAX_PACKET_SIZE) {
        //no end of packet symbol
        Serial.print("no end of packet, ");
        cmdBuf[packetBufCounter] = 0;
        Serial.print("buf: ");
        Serial.println(cmdBuf);

        packetBufCounter = 0;
        startPacketReading = false;
      } 
      else {
        if (cmd == '$') {
          //end of packet
          //parse packet
          cmdBuf[packetBufCounter] = 0;
          Serial.print("ok, buf: ");
          Serial.println(cmdBuf);

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
  mt.write(buf, 8);
  mt.flush();
  Serial.print("shield: ");
  for (int i = 0; i < 8; i++) {
    Serial.print(buf[i]);
    Serial.print(" ");
  }
  Serial.println();
}

void echoCheck() { // Timer2 interrupt calls this function every 24uS where you can check the ping status.
  // Don't do anything here!
  if (sonar.check_timer()) { // This is how you check to see if the ping was received.
    // Here's where you can add code.
    pingDistance = sonar.ping_result / US_ROUNDTRIP_CM;
    isObstacleForward = pingDistance < 30;
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




