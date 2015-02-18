/*
 */

#define FORWARD 1
#define BACKWARD 2
#define BRAKE 3
#define RELEASE 4

//Pin connected to ST_CP of 74HC595
int latchPin = 8;
//Pin connected to SH_CP of 74HC595
int clockPin = 12;
////Pin connected to DS of 74HC595
int dataPin = 11;

int pwmPins[4]  = {5, 6, 9, 10};

int buf[8];
int ind;
int pwmSpeed = 0;

uint8_t latch_state = 0;

void setup() {
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);  
  pinMode(clockPin, OUTPUT);
  
  for (ind = 0; ind < 4; ind++) {
    pinMode(pwmPins[ind], OUTPUT);
  }
}

void loop() {
  pwmSpeed = 100;
  updateSpeed();
  test();
  
  pwmSpeed = 255;
  updateSpeed();
  test();
}

void updateSpeed() {
  for (ind = 0; ind < 4; ind++) {
    pinMode(pwmPins[ind], OUTPUT);
    analogWrite(pwmPins[ind], pwmSpeed);
  }
} 

void test() {
  run(0,1,FORWARD);
  delay(1000);
  run(0,1,BACKWARD);
  delay(1000);
  run(0,1,RELEASE);
  delay(1000);
  run(2,3,FORWARD);
  delay(1000);
  run(2,3,BACKWARD);
  delay(1000);
  run(2,3,RELEASE);
  delay(1000);
  run(4,5,FORWARD);
  delay(1000);
  run(4,5,BACKWARD);
  delay(1000);
  run(4,5,RELEASE);
  delay(1000);
  run(6,7,FORWARD);
  delay(1000);
  run(6,7,BACKWARD);
  delay(1000);
  run(6,7,RELEASE);
  delay(1000);
  
  delay(3000);
  
  run(0,1,FORWARD);
  run(2,3,FORWARD);
  
  run(4,5,BACKWARD);
  run(6,7,BACKWARD);
  
  delay(3000);
  
  run(0,1,RELEASE);
  run(2,3,RELEASE);
  run(4,5,RELEASE);
  run(6,7,RELEASE);
  
  delay(1000);
}

void latch_tx(void) {
  uint8_t i;

  //LATCH_PORT &= ~_BV(LATCH);
  digitalWrite(latchPin, LOW);

  //SER_PORT &= ~_BV(SER);
  digitalWrite(dataPin, LOW);

  for (i=0; i<8; i++) {
    //CLK_PORT &= ~_BV(CLK);
    digitalWrite(clockPin, LOW);

    if (latch_state & _BV(7-i)) {
      //SER_PORT |= _BV(SER);
      digitalWrite(dataPin, HIGH);
    } else {
      //SER_PORT &= ~_BV(SER);
      digitalWrite(dataPin, LOW);
    }
    //CLK_PORT |= _BV(CLK);
    digitalWrite(clockPin, HIGH);
  }
  //LATCH_PORT |= _BV(LATCH);
  digitalWrite(latchPin, HIGH);
}

void run(uint8_t a, uint8_t b, uint8_t cmd) {
  switch (cmd) {
  case FORWARD:
    latch_state |= _BV(a);
    latch_state &= ~_BV(b); 
    latch_tx();
    break;
  case BACKWARD:
    latch_state &= ~_BV(a);
    latch_state |= _BV(b); 
    latch_tx();
    break;
  case RELEASE:
    latch_state &= ~_BV(a);     // A and B both low
    latch_state &= ~_BV(b); 
    latch_tx();
    break;
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


