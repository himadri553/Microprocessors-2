/*
  Lab 3: Audio-Driven Kinetic Sculpture with Infrared Control
  EECE.5520 - Microprocessor II and Embedded System Design
  
  Himadri Saha, Daniel Burns, Chris Worthley

  PIN MAPPING

  Lab3_src.ino:
  - Main code for lab 3
  - 

*/
/* Imports */
#include <Arduino.h>
#include <Wire.h>
#include <DS3231.h>

/* Vars */
DS3231 clock;
RTCDateTime dt;
int current_second;
int seconds_passed;

void setup() {
  Serial.begin(9600);

  // Pin setups

  // Clock setup
  clock.begin();
  clock.setDateTime(__DATE__, __TIME__);
  dt = clock.getDateTime();
  current_second = dt.second;

}

void loop() {
  // Get time, check if its been a min, and trigger fan
  dt = clock.getDateTime();
  seconds_passed = dt.second - current_second;
  if (seconds_passed > 30) {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("setting LED to high its been 30 sec");
  }
  else {
    digitalWrite(LED_BUILTIN, LOW); 
    Serial.println("setting LED to low");
  }

  delay(200);

} 