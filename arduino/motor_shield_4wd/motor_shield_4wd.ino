/*
 */
#define MOTOR_SHIELD_TRANSFER_SPEED 115200
 
int dir1Pins[4] = {2, 7, 12, 11};
int dir2Pins[4] = {4, 8, 13, 10};
int pwmPins[4]  = {3, 5, 6, 9};

int buf[8];
int ind;

void setup() {   
  Serial.begin(MOTOR_SHIELD_TRANSFER_SPEED); 

  for (ind = 0; ind < 4; ind++) {
    pinMode(dir1Pins[ind], OUTPUT);
    pinMode(dir2Pins[ind], OUTPUT);
    pinMode(pwmPins[ind], OUTPUT);
  }
}

void loop() {
  if (Serial.available() >= 8) {
    for (ind = 0; ind < 8; ind++) {
      buf[ind] = Serial.read();
    }
//    Serial.print("I received: ");
//    for (ind = 0; ind < 8; ind++) {
//      Serial.print(buf[ind]);
//      Serial.print("---");
//    }
//    Serial.println(" end");
    
    for (ind = 0; ind < 4; ind++) {
      updateMotor(dir1Pins[ind], dir2Pins[ind], pwmPins[ind], buf[ind], buf[ind + 4]);
    }
  }
}

void updateMotor(int dir1Pin, int dir2Pin, int pwmPin, int cmd, int speedVal) {
  switch (cmd) {
    case 97:
      //digitalWrite(dir1Pin, LOW);
      //digitalWrite(dir2Pin, LOW);
      digitalWrite(pwmPin, LOW);
      break;
    case 98:
      digitalWrite(dir1Pin, LOW);
      digitalWrite(dir2Pin, HIGH);
      analogWrite(pwmPin, speedVal);
      break;
    case 99:
      digitalWrite(dir1Pin, HIGH);
      digitalWrite(dir2Pin, LOW);
      analogWrite(pwmPin, speedVal);
      break;
    case 100:
      digitalWrite(dir1Pin, HIGH);
      digitalWrite(dir2Pin, HIGH);
      digitalWrite(pwmPin, HIGH);
      break;
  }
}


