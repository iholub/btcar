/*
 */
#include <NewPing.h>
#include <Wire.h>
#define SLAVE_ADDR 0x31 // Slave address, should be changed for other slaves

int hServoVal;
int vServoVal;

#define TRIGGER_PIN  4  // Arduino pin tied to trigger pin on ping sensor.
#define ECHO_PIN     7  // Arduino pin tied to echo pin on ping sensor.
#define MAX_DISTANCE 200 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

#define MAX_PACKET_SIZE 20
#define PACKET_TIMEOUT 100
char cmdBuf[MAX_PACKET_SIZE + 1];
unsigned long packetTimeStart = 0;
unsigned long packetTimeCurrent = 0;
boolean startPacketReading = false;
int packetBufCounter = 0;

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

unsigned int PING_PERIOD = 50; // How frequently are we going to send out a ping (in milliseconds). 50ms would be 20 times a second.
unsigned long previousPingTime;     // Holds the next ping time.
float pingDistance = 999.0;
float pingDistance2 = 999.0;
boolean isObstacleForward = false;

char cmd = 0;

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

// ampers start
#define AMP_OFFSET 2500
#define BATT_AMP_VOLTS_PER_AMP 66
#define BATT_AMP_PIN A1
#define BATT_AMP_READ_SPEED 50
unsigned long readBattAmpTimer;
float batteryAmps = 0.0;
// ampers end

// voltage start
#define BATT_VOLT_PIN A0
#define BATT_VOLT_READ_SPEED 50
unsigned long readBattVoltTimer;
float batteryVoltage = 0.0;
#define BATT_VOLT_R1 51000
#define BATT_VOLT_R2 47000
const float BATT_VOLT_VD = (BATT_VOLT_R1 + BATT_VOLT_R2) / BATT_VOLT_R2;
// voltage end

// info start
String infoStr;
static char outstr[15];
// info end

void setup() {
  Serial.begin(115200);
  Wire.begin();

  cmdBuf[MAX_PACKET_SIZE] = 0; //null terminated string

  pinMode(BATT_AMP_PIN, INPUT);
  //pinMode(BATT_VOLT_PIN, INPUT);

  previousPingTime = readBattVoltTimer = readBattAmpTimer = millis(); // Start now.

}

void loop() {
  sendPing();

  cmdUpdateMotor = false;
  cmdStopBeforeObstacle = false;
  cmdUpdateServoH = false;
  cmdUpdateServoV = false;

  doStopBeforeObstacle();

  readPacket();

  unsigned long currTime = millis();
  if (currTime - readBattAmpTimer >= BATT_AMP_READ_SPEED) {
    readBattAmpTimer = currTime;
    readBatteryAmps();
  }

  currTime = millis();
  if (currTime - readBattVoltTimer >= BATT_VOLT_READ_SPEED) {
    readBattVoltTimer = currTime;
    readBatteryVoltage();
  }

}

void doStopBeforeObstacle() {
  if (stopBeforeObstacle) {
    if (isObstacleForward && (lDir == 'f') && (rDir == 'f')) {
      stoppedBeforeObstacle = true;
      updateMotorShield('s', 's', 0, 0);
    }
    else {
      if (stoppedBeforeObstacle) {
        stoppedBeforeObstacle = false;
        //TODO fix
        updateMotorShield(lDir, rDir, lPwm, rPwm);
      }
    }
  }
}

void readPacket() {
  int avail = Serial.available();
  if (avail == 0) {
    return;
  }
  if (Serial.available() > 0) {
    cmd = Serial.read();
    if (startPacketReading) {
      packetTimeCurrent = millis();
      if (packetTimeCurrent - packetTimeStart > PACKET_TIMEOUT) {
        //timeout
        cmdBuf[packetBufCounter] = 0;
        packetBufCounter = 0;
        startPacketReading = false;
      }
      else if (packetBufCounter >= MAX_PACKET_SIZE) {
        //no end of packet symbol
        cmdBuf[packetBufCounter] = 0;
        packetBufCounter = 0;
        startPacketReading = false;
      }
      else {
        if (cmd == 'z') {
          //end of packet
          //parse packet
          cmdBuf[packetBufCounter] = 0;
          boolean parsedOk = parseCmdPacket();
          if (parsedOk) {
            updateSlave();
          }
          showInfo();

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
  }

}

void sendPing() {
  unsigned long currTime = millis();
  if (currTime - previousPingTime >= PING_PERIOD) {   // PING_PERIOD milliseconds since last ping, do another ping.
    previousPingTime = currTime;      // Set the next ping time.
    sonar.ping_timer(echoCheck); // Send out the ping, calls "echoCheck" function every 24uS where you can check the ping status.
  }
}

void updateSlave() {
  if (cmdUpdateMotor || cmdUpdateServoH || cmdUpdateServoV) {
    Wire.beginTransmission(SLAVE_ADDR);
    if (cmdUpdateMotor) {
      Wire.write(0x0C);

      Wire.write(dirToByte(lDir));
      Wire.write(lPwm);
      Wire.write(dirToByte(lDir));
      Wire.write(lPwm);

      Wire.write(dirToByte(rDir));
      Wire.write(rPwm);
      Wire.write(dirToByte(rDir));
      Wire.write(rPwm);
    }
    if (cmdUpdateServoH) {
      Wire.write(0x0A);
      Wire.write(hServoVal);
    }
    if (cmdUpdateServoV) {
      Wire.write(0x0B);
      Wire.write(vServoVal);
    }
    Wire.endTransmission();
  }
}

byte dirToByte(char cmd) {
  switch (cmd) {
    case 'f':
      return 1;
    case 'b':
      return 2;
    case 's':
      return 3;
  }
  //TODO brake, release
  return 4;
}

void updateMotorShield(char plDir, char prDir, int plPwm, int prPwm) {
  lDir = plDir;
  rDir = prDir;
  lPwm = plPwm;
  rPwm = prPwm;
  cmdUpdateMotor = true;
  updateSlave();
  cmdUpdateMotor = false;
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

void readBatteryVoltage() {
  batteryVoltage = (analogRead(BATT_VOLT_PIN) * 5.0 * BATT_VOLT_VD) / 1024.0;
}

void readBatteryAmps() {
  //TODO 1023 ???
  batteryAmps = ((((analogRead(BATT_AMP_PIN) / 1023.0) * 5000) - AMP_OFFSET) / BATT_AMP_VOLTS_PER_AMP);
}

void showInfo() {
  infoStr = "";
  infoStr += "uptime: ";
  infoStr += millis();

  dtostrf(batteryAmps, 7, 3, outstr);
  infoStr += ", amps: ";
  infoStr += outstr;

  dtostrf(batteryVoltage, 7, 3, outstr);
  infoStr += ", volts: ";
  infoStr += outstr;

  dtostrf(pingDistance, 7, 3, outstr);
  infoStr += ", ping: ";
  infoStr += outstr;

  Serial.println(infoStr);
}

