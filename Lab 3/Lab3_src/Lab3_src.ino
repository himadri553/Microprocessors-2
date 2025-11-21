/*
  Lab 3: Audio-Driven Kinetic Sculpture with Infrared Control
  EECE.5520 - Microprocessor II and Embedded System Design
  
  Himadri Saha, Daniel Burns, Chris Worthley

  PIN MAPPING
    D5 → L293D pin 1 (Enable 1,2 – PWM)
    D4 → L293D pin 2 (IN1)
    D3 → L293D pin 7 (IN2)
    D2 → Pushbutton (to GND, use INPUT_PULLUP)
    A4 → SDA  (I²C data to RTC)
    A5 → SCL  (I²C clock to RTC)
    LCD pins: RS=7, EN=8, D4=9, D5=10, D6=11, D7=12
    VCC → 5 V, GND → common ground (Arduino + motor + RTC + LCD)

  Lab3_src.ino:
  - Main code for lab 3
  - 

*/
#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal.h>
#include <avr/interrupt.h>

RTC_DS1307 rtc;
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);   // RS, EN, D4, D5, D6, D7

// Motor pins
const int ENA = 5;
const int IN1 = 4;
const int IN2 = 3;

// Direction button
const int BTN_DIR = 2;
bool dirCW = true;
bool lastButtonState = HIGH;
unsigned long lastDebounce = 0;
const unsigned long debounceDelay = 200;

// Sound sensor
const int soundPin = A0;
int soundValue = 0;  // peak-to-peak result

// Speed control
int pwmSpeed = 0;
String speedLabel = "0";

// Timing
const unsigned long RUN_DURATION = 30000;
bool motorRunning = false;
unsigned long motorStartTime = 0;

volatile bool updateDisplay = false;


void setup() {
  Serial.begin(9600);

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(BTN_DIR, INPUT_PULLUP);
  pinMode(soundPin, INPUT);

  // RTC setup
  Wire.begin();
  rtc.begin();
  // Uncomment ONCE to set RTC time, then re-comment
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  // LCD setup
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Step 7 Ready");
  delay(800);
  lcd.clear();

  // Timer1 → 1 Hz ISR
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  OCR1A = 15624;                      // 1 second at 16MHz/1024
  TCCR1B |= (1 << WGM12);             
  TCCR1B |= (1 << CS12) | (1 << CS10);
  TIMSK1 |= (1 << OCIE1A);            
  sei();
}


void loop() {

  // --- Button handling ---
  bool reading = digitalRead(BTN_DIR);
  if (reading != lastButtonState && millis() - lastDebounce > debounceDelay) {
    lastDebounce = millis();
    if (reading == LOW) {
      dirCW = !dirCW;
      if (motorRunning)
        applyDirection();
    }
  }
  lastButtonState = reading;

  // --- Peak-to-peak audio measurement ---
  soundValue = readSoundP2P();

  // --- Convert P2P level to motor speed ---
  if (soundValue < 40) {
    pwmSpeed = 0;
    speedLabel = "0";
  }
  else if (soundValue <120) {
    pwmSpeed = 128;
    speedLabel = "1/2";
  }
  else if (soundValue < 200) {
    pwmSpeed = 192;
    speedLabel = "3/4";
  }
  else {
    pwmSpeed = 255;
    speedLabel = "Full";
  }

  // REAL-TIME MOTOR SPEED
  if (motorRunning)
      analogWrite(ENA, pwmSpeed);


  // RTC-driven schedule: start motor every minute
  DateTime now = rtc.now();
  if (now.second() == 0 && !motorRunning) {
    startMotor();
    motorRunning = true;
    motorStartTime = millis();
  }

  if (motorRunning && millis() - motorStartTime >= RUN_DURATION) {
    stopMotor();
    motorRunning = false;
  }

  // --- LCD update (from ISR flag) ---
  if (updateDisplay) {
    updateDisplay = false;
    updateLCD();
  }
}


// TIMER1 ISR (fires every 1 second)

ISR(TIMER1_COMPA_vect) {
  updateDisplay = true;
}

// MOTOR FUNCTIONS

void startMotor() {
  applyDirection();
  analogWrite(ENA, pwmSpeed);
}

void stopMotor() {
  analogWrite(ENA, 0);
}

void applyDirection() {
  if (dirCW) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
  } else {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
  }
}

// LCD UPDATE FUNCTION

void updateLCD() {
  DateTime now = rtc.now();

  char buf[9];
  sprintf(buf, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());

  lcd.setCursor(0, 0);
  lcd.print(buf);
  lcd.print("   ");

  lcd.setCursor(0, 1);
  lcd.print("Dir:");
  lcd.print(dirCW ? "C " : "CC");
  lcd.print(" Spd:");
  lcd.print(speedLabel);
  lcd.print("   ");
}


// AUDIO SAMPLING: PEAK-TO-PEAK (P2P)

int readSoundP2P()
{
  unsigned long startTime = millis();
  int maxVal = 0;
  int minVal = 1023;

  // 50 ms sampling window
  while (millis() - startTime < 50) {
    int sample = analogRead(soundPin);
    if (sample > maxVal) maxVal = sample;
    if (sample < minVal) minVal = sample;
  }

  return (maxVal - minVal);
}