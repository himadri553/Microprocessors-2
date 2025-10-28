/*
  Lab 2: Game Control with Joy Stick, Gyro and Accelerometer
  EECE.5520 - Microprocessor II and Embedded System Design
  
  Himadri Saha, Daniel Burns, Chris Worthley

  PIN MAPPING
  Active Buzzer: PWM13
  MPU-6050 SDA: 20
  MPU-6050 SCL: 21
  Joystick horz: A1
  Joystick vert: A0
  Joystick SW: 2

  Lab2_main.ino:
  - Main runner file for lab 2
  - Functionality:
    * Joystick for snake direction
    * MPU sensor for snake movement???
    * buzzer beeps when apple is eaten
    * "shaking" from accleometer used to setup
  - Communication protocol:

*/
/* Imports */
#include <Arduino.h>
#include <Wire.h>

/* Pin maps */
#define BUZZER 13
#define JOY_X A1
#define JOY_Y A0
#define JOY_SW 2

/* Other Vars */
#define JOY_deadZone 200    // 
#define JOY_detRange 512    // 
int controllerMode = 2;     // 1: Joystick, 2: Gyro
int16_t GyX, GyY, GyZ;      // Raw Gyro values
const int MPU_addr = 0x69;  // I2C address of the MPU-6050  
#define GYRO_threshold 1000 // prevent noise on gyro

/* Helper functions */
void beep() {
  digitalWrite(BUZZER, HIGH);
  delay(100);
  digitalWrite(BUZZER, LOW);
}

void send_joystick_command(int x_val, int y_val) {
  if (x_val < JOY_detRange - JOY_deadZone) {
    Serial.write('d');
  }
  else if (x_val > JOY_detRange + JOY_deadZone) {
    Serial.write('a');
  }
  else if (y_val < JOY_detRange - JOY_deadZone) {
    Serial.write('s');
  }
  else if (y_val > JOY_detRange + JOY_deadZone) {
    Serial.write('w');
  }
}

/* Runner functions */
void setup() {
  // Pin setup and serial
  Wire.begin();
  Serial.begin(9600);
  pinMode(BUZZER, OUTPUT);
  pinMode(JOY_SW, INPUT_PULLUP);

  // Setup gyro   
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);   // Power management register
  Wire.write(0);      // Wake up MPU6050
  Wire.endTransmission(true);

  // Prompt user for controller mode


}

void loop() {
  switch(controllerMode) {
    case 1:
      /* Joystick Mode: Read joystick and send command */
      Serial.println("Going into joystick mode...");
      int joyX_val = analogRead(JOY_X);
      int joyY_val = analogRead(JOY_Y);
      send_joystick_command(joyX_val, joyY_val);

      break;

    case 2:
      /* Gyro Mode:  */
      // Request gyro registers (start at 0x43)
      Wire.beginTransmission(MPU_addr);
      Wire.write(0x43);
      Wire.endTransmission(false);
      Wire.requestFrom(MPU_addr, 6, true);  

      GyX = Wire.read() << 8 | Wire.read();  
      GyY = Wire.read() << 8 | Wire.read();  
      GyZ = Wire.read() << 8 | Wire.read();  

      Serial.print("GyX="); Serial.print(GyX);
      Serial.print(" | GyY="); Serial.print(GyY);
      Serial.print(" | GyZ="); Serial.println(GyZ);

      /*
            // Determine direction based on gyro tilt
      // Positive X tilt → move right, Negative X tilt → move left
      // Positive Y tilt → move up, Negative Y tilt → move down
      if (GyX > GYRO_threshold)
        Serial.write('d');   // tilt right
      else if (GyX < -GYRO_threshold)
        Serial.write('a');   // tilt left
      else if (GyY > GYRO_threshold)
        Serial.write('w');   // tilt up
      else if (GyY < -GYRO_threshold)
        Serial.write('s');   // tilt down
      */
        
      break;

    default:
      Serial.println("Deafulting to joystick mode");
      controllerMode = 1;
  }

  /*
      // Beep on apple collect
  if (Serial.available()) {
    char ch = Serial.read();
    if (ch == 'E') beep();
  }
  */

  delay(500);
}
