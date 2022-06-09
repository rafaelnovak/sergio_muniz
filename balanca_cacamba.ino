/**

   HX711 library for Arduino - example file
   https://github.com/bogde/HX711

   MIT License
   (c) 2018 Bogdan Necula

**/

///////////////////////////////BALANCA/////////////////////////////////////////////////////
#include "HX711.h"

// HX711 circuit wiring
#define LOADCELL_DOUT_PIN A2 //(branco)
#define LOADCELL_SCK_PIN A3 //(roxo)
#define LED 12

HX711 scale;

///////////////////////////////SERVO/////////////////////////////////////////////////////
#include <Servo.h>
Servo myservo;  // create servo object to control a servo
// twelve servo objects can be created on most boards

int pos;    // variable to store the servo position
int pos_start; 

void setup() {
///////////////////////////////BALANCA/////////////////////////////////////////////////////

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  scale.set_scale(2280.f);    // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();               // reset the scale to 0

  //Serial.println("After setting up the scale:");
  pinMode(LED, OUTPUT);

///////////////////////////////SERVO/////////////////////////////////////////////////////
  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
  pos_start = myservo.read();
  for (pos = pos_start; pos <= 90; pos=+1){
    myservo.write(pos);
    delay(15);
  }
}

void loop() {

  ///////////////////////////////BALANCA////////////////////////////////////////////////////
  float leitura;
  //Serial.print("one reading:\t");
  //Serial.print(scale.get_units()*(-1), 1);
  //Serial.print("\t| average:\t");
  leitura = scale.get_units(1)*(-1);
  //Serial.println(leitura, 1);
  scale.power_down();             // put the ADC in sleep mode
  delay(10);
  scale.power_up();
  digitalWrite(LED,HIGH);

  ///////////////////////////////SERVO/////////////////////////////////////////////////////
  
  if (leitura > 1.5) {
    for (pos = 90; pos >= 0; pos -= 1) {
    // in steps of 1 degree
    myservo.write(pos);              
    delay(50);                       
  }
  delay(4000);
  for (pos = 0; pos <= 90; pos += 1) { 
    myservo.write(pos);              
    delay(50);                       
  }
}

}
