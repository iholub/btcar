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

#define MAX_PACKET_SIZE 5
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
boolean stopBeforeObstacle = false;
unsigned long pingTime1 = 0;
unsigned long pingTime2 = 0;


SoftwareSerial bt(BLUETOOTH_RX, BLUETOOTH_TX); // RX, TX
SoftwareSerial mt(MOTOR_SHIELD_RX, MOTOR_SHIELD_TX); // RX, TX

char cmd = 0;
char dirCmd = 0;
byte buf[8];
byte cmds[4];
byte sp = 255;

void setup() {   
  Serial.begin(9600);  
  bt.begin(38400);
  mt.begin(9600);
  bt.listen();  

  pingTimer = millis(); // Start now.
}

void loop() {
  // Notice how there's no delays in this sketch to allow you to do other processing in-line while doing distance pings.
  if (millis() >= pingTimer) {   // pingSpeed milliseconds since last ping, do another ping.
    pingTimer += pingSpeed;      // Set the next ping time.
    sonar.ping_timer(echoCheck); // Send out the ping, calls "echoCheck" function every 24uS where you can check the ping status.
  }
    if (stopBeforeObstacle) {
       if (isObstacleForward && (dirCmd == 'w')) {
	 cmds[0] = 100;
	 cmds[1] = 100;
	 cmds[2] = 100;
	 cmds[3] = 100;
         updateMotors();
       } else {
         //TODO 
       }
    }

pingTime2 = millis();
if (pingTime2 - pingTime1 > 500) {
  pingTime1 = pingTime2;
  bt.print("distance: ");
  bt.println(pingDistance);
}
  if (bt.available() > 0) {
    cmd = bt.read();
    //Serial.print(cmd);
    bt.print("I received: ");
    bt.println(cmd);
    if (startPacketReading) {
      packetTimeCurrent = millis();
      if (packetTimeCurrent - packetTimeStart > 15000) {
        //timeout
        Serial.print("timeout, ");
        cmdBuf[packetBufCounter] = 0;
        Serial.print("buf: ");
        Serial.println(cmdBuf);
        
        packetBufCounter = 0;
        startPacketReading = false;
      } else if (packetBufCounter >= MAX_PACKET_SIZE) {
        //no end of packet symbol
        Serial.print("no end of packet, ");
        cmdBuf[packetBufCounter] = 0;
        Serial.print("buf: ");
        Serial.println(cmdBuf);
        
        packetBufCounter = 0;
        startPacketReading = false;

      } else {
        if (cmd == '$') {
          //end of packet
          //parse packet
          cmdBuf[packetBufCounter] = 0;
          Serial.print("ok, buf: ");
          Serial.println(cmdBuf);
          
          packetBufCounter = 0;
          startPacketReading = false;
        } else {
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
    switch (cmd) {
    case 'w'://forward
      dirCmd = cmd;
      cmds[0] = 98;
      cmds[1] = 98;
      cmds[2] = 98;
      cmds[3] = 98;
      break;
    case 'a'://rotate left
      dirCmd = cmd;
      cmds[0] = 99;
      cmds[1] = 98;
      cmds[2] = 99;
      cmds[3] = 98;
      break;
    case 's'://stop
      dirCmd = cmd;
      cmds[0] = 97;
      cmds[1] = 97;
      cmds[2] = 97;
      cmds[3] = 97;
      break;
    case 'd'://rotate right
      dirCmd = cmd;
      cmds[0] = 98;
      cmds[1] = 99;
      cmds[2] = 98;
      cmds[3] = 99;
      break;
    case 'z'://backwards
      dirCmd = cmd;
      cmds[0] = 99;
      cmds[1] = 99;
      cmds[2] = 99;
      cmds[3] = 99;
      break;
    case 'p'://breakes
      dirCmd = cmd;
      cmds[0] = 100;
      cmds[1] = 100;
      cmds[2] = 100;
      cmds[3] = 100;
      break;
    case 'u'://speed max
      sp = 255;
      break;
    case 'h'://speed middle
      sp = 175;
      break;
    case 'b'://speed low
      sp = 100;
      break;
    case 'o':
      stopBeforeObstacle = !stopBeforeObstacle;
      if (stopBeforeObstacle) {
        bt.println("obstacle on");
      } else {
        bt.println("obstacle off");
      }
      break;
    }
    updateMotors();
  }
}

void updateMotors() {
  buf[0] = cmds[0];
  buf[1] = cmds[1];
  buf[2] = cmds[2];
  buf[3] = cmds[3];
  buf[4] = sp;
  buf[5] = sp;
  buf[6] = sp;
  buf[7] = sp;
  mt.write(buf, 8);
  mt.flush();
  //Serial.println("updateMotors end");
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

