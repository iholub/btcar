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

const int m1Dir1Pin = 4;
const int m1Dir2Pin = 5;
const int m1PwmPin = 3;

const int m2Dir1Pin = 7;
const int m2Dir2Pin = 8;
const int m2PwmPin = 6;

char cmd = 0;
char dirCmd = 0;
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
          updateMotor(m1Dir1Pin, m1Dir2Pin, m1PwmPin, 3);
          updateMotor(m2Dir1Pin, m2Dir2Pin, m2PwmPin, 3);
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
      updateMotor(m1Dir1Pin, m1Dir2Pin, m1PwmPin, 1);
      updateMotor(m2Dir1Pin, m2Dir2Pin, m2PwmPin, 1);
      break;
    case 'a'://rotate left
      dirCmd = cmd;
      updateMotor(m1Dir1Pin, m1Dir2Pin, m1PwmPin, 2);
      updateMotor(m2Dir1Pin, m2Dir2Pin, m2PwmPin, 1);
      break;
    case 's'://stop
      dirCmd = cmd;
      updateMotor(m1Dir1Pin, m1Dir2Pin, m1PwmPin, 0);
      updateMotor(m2Dir1Pin, m2Dir2Pin, m2PwmPin, 0);
      break;
    case 'd'://rotate right
      dirCmd = cmd;
      updateMotor(m1Dir1Pin, m1Dir2Pin, m1PwmPin, 1);
      updateMotor(m2Dir1Pin, m2Dir2Pin, m2PwmPin, 2);
      break;
    case 'z'://backwards
      dirCmd = cmd;
      updateMotor(m1Dir1Pin, m1Dir2Pin, m1PwmPin, 2);
      updateMotor(m2Dir1Pin, m2Dir2Pin, m2PwmPin, 2);
      break;
    case 'p'://breakes
      dirCmd = cmd;
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

void echoCheck() { // Timer2 interrupt calls this function every 24uS where you can check the ping status.
  // Don't do anything here!
  if (sonar.check_timer()) { // This is how you check to see if the ping was received.
    // Here's where you can add code.
    pingDistance = sonar.ping_result / US_ROUNDTRIP_CM;
    isObstacleForward = pingDistance < 30;
  }
  // Don't do anything here!
}

