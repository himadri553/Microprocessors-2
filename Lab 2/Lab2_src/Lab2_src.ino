/*
  Lab 2: Game Control with Joy Stick, Gyro and Accelerometer
  EECE.5520 - Microprocessor II and Embedded System Design
  
  Himadri Saha, Daniel Burns, Chris Worthley

  PIN MAPPING
  Active Buzzer: PWM13
  MPU-6050 SDA: A3
  MPU-6050 SCL: A2
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

/* Pin maps */
#define BUZZER 13
#define GYRO_SDA A3
#define GYRO_SCL A2
#define JOY_X A1
#define JOY_Y A0
#define JOY_SW 2

/* Other Vars */
#define JOY_deadZone 200  // 
#define JOY_detRange 512  // 
int controllerMode = 1;   // 1: Joystick, 2: Gyro
int16_t GyX, GyY, GyZ;    // Raw Gyro values

/* Helper functions */
void beep() {
  digitalWrite(buzzer_pin, HIGH);
  delay(100);
  digitalWrite(buzzer_pin, LOW);
}

void send_joystick_command(int x_val, int y_val) {
  // Prompt user to select controller mode (gyro or joystick)
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
  Serial.begin(9600);
  pinMode(BUZZER, OUTPUT);
  pinMode(JOY_SW, INPUT_PULLUP);
  
}

void loop() {
  switch(controllerMode) {
    case 1:
      /* Joystick Mode: Read joystick and send command */
      int joyX_val = analogRead(JOY_X);
      int joyY_val = analogRead(JOY_Y);
      send_joystick_command(JOY_X, JOY_Y);

      break;

    case 2:
      /* Gyro Mode:  */

      break;

    default:
      Serial.println("Deafulting to joystick mode");
      controllerMode = 1;
  }

  // Beep on apple collect
  if (Serial.available()) {
    char ch = Serial.read();
    if (ch == 'E') beep();
  }

}
