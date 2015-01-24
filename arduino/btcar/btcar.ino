/*
 */
#include <SoftwareSerial.h>
#include <NewPing.h>

#define TRIGGER_PIN  12  // Arduino pin tied to trigger pin on ping sensor.
#define ECHO_PIN     11  // Arduino pin tied to echo pin on ping sensor.
#define MAX_DISTANCE 200 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

unsigned int pingSpeed = 50; // How frequently are we going to send out a ping (in milliseconds). 50ms would be 20 times a second.
unsigned long pingTimer;     // Holds the next ping time.
float pingDistance = 999.0;
boolean isObstacleForward = false;
boolean stopBeforeObstacle = false;
unsigned long pingTime1 = 0;
unsigned long pingTime2 = 0;

SoftwareSerial bt(10, 9); // RX, TX
SoftwareSerial mt(2, 3); // RX, TX

char cmd = 0;
char dirCmd = 0;
byte buf[8];
byte cmds[4];
byte sp = 255;

void setup() {   
  Serial.begin(9600);  
  bt.begin(38400);
  mt.begin(9600);  

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
      dirCmd = cmd;
      cmds[0] = 98;
      cmds[1] = 98;
      cmds[2] = 98;
      cmds[2] = 98;
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
    }
	
  }
}

void updateMotor(int motorInd, byte cmd) {
  buf[0] = cmds[0];
  buf[1] = cmds[1];
  buf[2] = cmds[2];
  buf[3] = cmds[3];
  buf[4] = sp;
  buf[5] = sp;
  buf[6] = sp;
  buf[7] = sp;
  mt.write(buf, 8);
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

