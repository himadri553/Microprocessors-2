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

#include <Arduino.h>
#include <Wire.h>

/* Pins */
#define BUZZER 13
#define JOY_X A1
#define JOY_Y A0
#define JOY_SW 2

/* Joystick */
#define JOY_CENTER 512
#define JOY_DEADZONE 150

/* MPU-6050 */
const int MPU_addr = 0x68;   // use 0x69 only if AD0 is tied HIGH
#define TILT_THRESHOLD 0.8  // fraction of 1 g (~0.2 g) tilt to trigger
#define SHAKE_THRESHOLD 32700
/* State */
int controllerMode = 1;      // 1 = joystick, 2 = gyro
bool modeSelected = false;

/* Raw sensor data */
int16_t AcX, AcY, AcZ;
int16_t GyX, GyY, GyZ;

void beep() {
  digitalWrite(BUZZER, HIGH);
  delay(100);
  digitalWrite(BUZZER, LOW);
}

/* Joystick control */
void send_joystick_command() {
  int x = analogRead(JOY_X);
  int y = analogRead(JOY_Y);

  if (x > JOY_CENTER + JOY_DEADZONE)
    Serial.write('a'); // left
  else if (x < JOY_CENTER - JOY_DEADZONE)
    Serial.write('d'); // right
  else if (y > JOY_CENTER + JOY_DEADZONE)
    Serial.write('s'); // up
  else if (y < JOY_CENTER - JOY_DEADZONE)
    Serial.write('w'); // down
}

/* Gyro/Accelerometer control */
void send_gyro_command() {
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);   // start at accel registers
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr, 14, true);

  AcX = (Wire.read() << 8) | Wire.read();
  AcY = (Wire.read() << 8) | Wire.read();
  AcZ = (Wire.read() << 8) | Wire.read();
  GyX = (Wire.read() << 8) | Wire.read();
  GyY = (Wire.read() << 8) | Wire.read();
  GyZ = (Wire.read() << 8) | Wire.read();

  // Convert to g’s (assuming default ±2 g range)
  float ax = AcX / 16384.0;
  float ay = AcY / 16384.0;
  float az = AcZ / 16384.0;

  // Detect tilt direction
  if (ax >  TILT_THRESHOLD)  Serial.write('a');       // tilt right
  else if (ax < -TILT_THRESHOLD) Serial.write('d');   // tilt left
  else if (ay >  TILT_THRESHOLD)  Serial.write('s');  // tilt up
  else if (ay < -TILT_THRESHOLD) Serial.write('w');   // tilt down
  
  // --- Detect shake gesture ---
  // If any gyro axis changes quickly → considered a shake
  if (abs(GyX) > SHAKE_THRESHOLD || abs(GyY) > SHAKE_THRESHOLD || abs(GyZ) > SHAKE_THRESHOLD) {
    Serial.write('X'); // signal to Python
  }

  delay(50);
}

void setup() {
  Wire.begin();
  Serial.begin(9600);
  pinMode(BUZZER, OUTPUT);
  pinMode(JOY_SW, INPUT_PULLUP);

  // Wake MPU-6050
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  Serial.println("Arduino ready. Send '1' (joystick) or '2' (gyro).");
}

void loop() {
  // Handle serial from Python
  while (Serial.available() > 0) {
    char ch = Serial.read();
    if (ch == '1') {
      controllerMode = 1;
      modeSelected = true;
      Serial.println("MODE: JOYSTICK");
    } else if (ch == '2') {
      controllerMode = 2;
      modeSelected = true;
      Serial.println("MODE: GYRO");
    } else if (ch == 'E') {
      beep();
    }
  }

  if (!modeSelected) controllerMode = 1;

  if (controllerMode == 1)
    send_joystick_command();
  else
    send_gyro_command();

  delay(100);
}