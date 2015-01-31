/*
 */
#include <SoftwareSerial.h>

#define BLUETOOTH_RX 10
#define BLUETOOTH_TX 9

SoftwareSerial bt(BLUETOOTH_RX, BLUETOOTH_TX); // RX, TX

char cmd = 0;

void setup() {   
  Serial.begin(9600);  
  bt.begin(38400);
}

void loop() {
  if (bt.available() > 0) {
    cmd = bt.read();
    Serial.print(cmd);
//    bt.print("I received: ");
//    bt.println(cmd);
  }
}

