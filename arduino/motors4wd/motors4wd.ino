/*
 */
 
const int m1Dir1Pin = 2;
const int m1Dir2Pin = 4;
const int m1PwmPin = 3;

const int m2Dir1Pin = 12;
const int m2Dir2Pin = 13;
const int m2PwmPin = 5;

const int m3Dir1Pin = 7;
const int m3Dir2Pin = 8;
const int m3PwmPin = 6;

const int m4Dir1Pin = 11;
const int m4Dir2Pin = 10;
const int m4PwmPin = 9;

char cmd = 0;
int speedVal = 100;

void setup() {   
  Serial.begin(9600);  

  pinMode(m1Dir1Pin,OUTPUT);
  pinMode(m1Dir2Pin,OUTPUT);
  pinMode(m1PwmPin,OUTPUT);

  pinMode(m2Dir1Pin,OUTPUT);
  pinMode(m2Dir2Pin,OUTPUT);
  pinMode(m2PwmPin,OUTPUT);

  pinMode(m3Dir1Pin,OUTPUT);
  pinMode(m3Dir2Pin,OUTPUT);
  pinMode(m3PwmPin,OUTPUT);

  pinMode(m4Dir1Pin,OUTPUT);
  pinMode(m4Dir2Pin,OUTPUT);
  pinMode(m4PwmPin,OUTPUT);

  updateSpeedAll();
}

void loop() {
//  if (Serial.available()) {
//    char ccc = Serial.read();
//     Serial.write(ccc);
//     Serial.write(ccc + 1);
//     return;
//  }
  
  if (Serial.available() > 0) {
    cmd = Serial.read();
    Serial.print("I received: ");
    Serial.println(cmd);
 // say what you got:
                //Serial.print("I received: ");
                //Serial.println(incomingByte, DEC);
    switch (cmd) {
      case 'w':
        updateMotor(m1Dir1Pin, m1Dir2Pin, m1PwmPin, 1);
        break;
      case 's':
        updateMotor(m1Dir1Pin, m1Dir2Pin, m1PwmPin, 0);
        break;
      case 'z':
        updateMotor(m1Dir1Pin, m1Dir2Pin, m1PwmPin, 2);
        break;
        
      case 'e':
        updateMotor(m2Dir1Pin, m2Dir2Pin, m2PwmPin, 1);
        break;
      case 'd':
        updateMotor(m2Dir1Pin, m2Dir2Pin, m2PwmPin, 0);
        break;
      case 'x':
        updateMotor(m2Dir1Pin, m2Dir2Pin, m2PwmPin, 2);
        break;


      case 'r':
        updateMotor(m3Dir1Pin, m3Dir2Pin, m3PwmPin, 1);
        break;
      case 'f':
        updateMotor(m3Dir1Pin, m3Dir2Pin, m3PwmPin, 0);
        break;
      case 'c':
        updateMotor(m3Dir1Pin, m3Dir2Pin, m3PwmPin, 2);
        break;


      case 't':
        updateMotor(m4Dir1Pin, m4Dir2Pin, m4PwmPin, 1);
        break;
      case 'g':
        updateMotor(m4Dir1Pin, m4Dir2Pin, m4PwmPin, 0);
        break;
      case 'v':
        updateMotor(m4Dir1Pin, m4Dir2Pin, m4PwmPin, 2);
        break;


      case 'i':
        updateMotor(m1Dir1Pin, m1Dir2Pin, m1PwmPin, 1);
        updateMotor(m2Dir1Pin, m2Dir2Pin, m2PwmPin, 1);
        updateMotor(m3Dir1Pin, m3Dir2Pin, m3PwmPin, 1);
        updateMotor(m4Dir1Pin, m4Dir2Pin, m4PwmPin, 1);
        break;
      case 'j':
        updateMotor(m1Dir1Pin, m1Dir2Pin, m1PwmPin, 2);
        updateMotor(m2Dir1Pin, m2Dir2Pin, m2PwmPin, 1);
        updateMotor(m3Dir1Pin, m3Dir2Pin, m3PwmPin, 2);
        updateMotor(m4Dir1Pin, m4Dir2Pin, m4PwmPin, 1);
        break;
      case 'k':
        updateMotor(m1Dir1Pin, m1Dir2Pin, m1PwmPin, 1);
        updateMotor(m2Dir1Pin, m2Dir2Pin, m2PwmPin, 2);
        updateMotor(m3Dir1Pin, m3Dir2Pin, m3PwmPin, 1);
        updateMotor(m4Dir1Pin, m4Dir2Pin, m4PwmPin, 2);
        break;
      case 'n':
      case 'm':
        updateMotor(m1Dir1Pin, m1Dir2Pin, m1PwmPin, 2);
        updateMotor(m2Dir1Pin, m2Dir2Pin, m2PwmPin, 2);
        updateMotor(m3Dir1Pin, m3Dir2Pin, m3PwmPin, 2);
        updateMotor(m4Dir1Pin, m4Dir2Pin, m4PwmPin, 2);
        break;
      case 'p':
        updateMotor(m1Dir1Pin, m1Dir2Pin, m1PwmPin, 3);
        updateMotor(m2Dir1Pin, m2Dir2Pin, m2PwmPin, 3);
        updateMotor(m3Dir1Pin, m3Dir2Pin, m3PwmPin, 3);
        updateMotor(m4Dir1Pin, m4Dir2Pin, m4PwmPin, 3);
        break;
        
      case 'u':
        speedVal = 255;
        updateSpeedAll();
        break;
      case 'h':
        speedVal = 175;
        updateSpeedAll();
        break;
      case 'b':
        speedVal = 100;
        updateSpeedAll();
        break;
      case 'o':
        speedVal = 0;
        updateSpeedAll();
        break;
        
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
  analogWrite(m3PwmPin, speedVal);
  analogWrite(m4PwmPin, speedVal);
}

void motorsStop() {
  updateMotor(m1Dir1Pin, m1Dir2Pin, m1PwmPin, 0);
  updateMotor(m2Dir1Pin, m2Dir2Pin, m2PwmPin, 0);
  updateMotor(m3Dir1Pin, m3Dir2Pin, m3PwmPin, 0);
  updateMotor(m4Dir1Pin, m4Dir2Pin, m4PwmPin, 0);
}










void test() {
  updateMotor(m1Dir1Pin, m1Dir2Pin, m1PwmPin, 1);
  updateMotor(m2Dir1Pin, m2Dir2Pin, m2PwmPin, 1);
  updateMotor(m3Dir1Pin, m3Dir2Pin, m3PwmPin, 1);
  updateMotor(m4Dir1Pin, m4Dir2Pin, m4PwmPin, 1);
  delay(5000);



  updateMotor(m1Dir1Pin, m1Dir2Pin, m1PwmPin, 2);
  delay(3000);
  
  updateMotor(m2Dir1Pin, m2Dir2Pin, m2PwmPin, 2);
  delay(3000);

  updateMotor(m3Dir1Pin, m3Dir2Pin, m3PwmPin, 2);
  delay(3000);

  updateMotor(m4Dir1Pin, m4Dir2Pin, m4PwmPin, 2);
  delay(3000);



  updateMotor(m1Dir1Pin, m1Dir2Pin, m1PwmPin, 0);
  delay(3000);
  
  updateMotor(m2Dir1Pin, m2Dir2Pin, m2PwmPin, 0);
  delay(3000);

  updateMotor(m3Dir1Pin, m3Dir2Pin, m3PwmPin, 0);
  delay(3000);

  updateMotor(m4Dir1Pin, m4Dir2Pin, m4PwmPin, 0);
  delay(3000);


  updateMotor(m1Dir1Pin, m1Dir2Pin, m1PwmPin, 3);
  updateMotor(m2Dir1Pin, m2Dir2Pin, m2PwmPin, 3);
  updateMotor(m3Dir1Pin, m3Dir2Pin, m3PwmPin, 3);
  updateMotor(m4Dir1Pin, m4Dir2Pin, m4PwmPin, 3);
  delay(5000);

  motorsStop();
  delay(5000);
}

