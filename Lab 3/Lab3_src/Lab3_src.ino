/*
  Lab 3: Audio-Driven Kinetic Sculpture with Infrared Control
  EECE.5520 - Microprocessor II and Embedded System Design
  
  Himadri Saha, Daniel Burns, Chris Worthley

  ---------------------------------------------------------------------------
  PIN MAPPING (DOUBLE-CHECK YOUR WIRING)
  ---------------------------------------------------------------------------

  --- MOTOR + L293D ---
    D5 → L293D pin 1 (Enable – PWM)
    D4 → L293D pin 2 (IN1)
    D3 → L293D pin 7 (IN2)
    Motor → L293D OUT1 + OUT2 (pins 3 & 6)
    L293D VCC2 (pin 8) → Motor supply (5–9V)
    L293D VCC1 (pin 16) → 5V
    All L293D GND pins → GND

  --- BUTTON ---
    D2 → Button → GND (INPUT_PULLUP)

  --- SOUND SENSOR ---
    VCC → 5V
    GND → GND
    AO  → A0

  --- RTC (DS1307) ---
    A4 → SDA
    A5 → SCL
    VCC → 5V
    GND → GND

  --- LCD (HD44780 16×2) ---
    1 (VSS)  → GND
    2 (VDD)  → 5V
    3 (VO)   → GND or pot wiper
    4 (RS)   → D7
    5 (RW)   → GND
    6 (EN)   → D8
    11 (D4)  → D9
    12 (D5)  → D10
    13 (D6)  → D11
    14 (D7)  → D12
    15 (LED+) → 5V
    16 (LED–) → GND

  ALL grounds must be common.
  ---------------------------------------------------------------------------
*/

#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal.h>
#include <avr/interrupt.h>

RTC_DS1307 rtc;
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

// Motor pins
const int ENA = 5;
const int IN1 = 4;
const int IN2 = 3;

// Button
const int BTN_DIR = 2;
bool dirCW = true;
bool lastButtonState = HIGH;
unsigned long lastDebounce = 0;
const unsigned long debounceDelay = 200;

// Sound sensor
const int soundPin = A0;
int soundValue = 0;

// Motor speed control
int pwmSpeed = 0;
String speedLabel = "0";

// Motor timing
const unsigned long RUN_DURATION = 30000;
bool motorRunning = false;
unsigned long motorStartTime = 0;

// Software clock via Timer1
volatile bool updateDisplay = false;   
volatile int hh, mm, ss;

DateTime startTime;


// -----------------------------------------------------------------------------
// SETUP
// -----------------------------------------------------------------------------
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

  // Read time ONCE
  startTime = rtc.now();
  hh = startTime.hour();
  mm = startTime.minute();
  ss = startTime.second();

  // LCD setup
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("Step 7 Ready");
  delay(800);
  lcd.clear();

  // Timer1 → 1Hz interrupt
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 15624;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS12) | (1 << CS10);
  TIMSK1 |= (1 << OCIE1A);
  sei();
}



// -----------------------------------------------------------------------------
// LOOP
// -----------------------------------------------------------------------------
void loop() {

  // --- BUTTON ---
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

  // --- SOUND ---
  soundValue = readSoundP2P();

  // ---------------------------------------------------------------------------
  // VERY LOW OUTPUT SOUND SENSOR THRESHOLDS
  // ---------------------------------------------------------------------------
  if (soundValue < 3) {
    pwmSpeed = 0;
    speedLabel = "0";
  }
  else if (soundValue < 10) {
    pwmSpeed = 128;
    speedLabel = "1/2";
  }
  else if (soundValue < 20) {
    pwmSpeed = 192;
    speedLabel = "3/4";
  }
  else {
    pwmSpeed = 255;
    speedLabel = "Full";
  }
  // ---------------------------------------------------------------------------

  // Apply PWM live
  if (motorRunning)
    analogWrite(ENA, pwmSpeed);

  // START MOTOR once per minute at second 0
  if (ss == 0 && !motorRunning) {
    startMotor();
    motorRunning = true;
    motorStartTime = millis();
  }

  // STOP MOTOR after 30s
  if (motorRunning && millis() - motorStartTime >= RUN_DURATION) {
    stopMotor();
    motorRunning = false;
  }

  // LCD update
  if (updateDisplay) {
    updateDisplay = false;
    updateLCD();
  }
}



// -----------------------------------------------------------------------------
// TIMER1 ISR (1 second)
// -----------------------------------------------------------------------------
ISR(TIMER1_COMPA_vect) {
  ss++;
  if (ss >= 60) { ss = 0; mm++; }
  if (mm >= 60) { mm = 0; hh++; }
  if (hh >= 24) { hh = 0; }

  updateDisplay = true;
}



// -----------------------------------------------------------------------------
// MOTOR FUNCTIONS
// -----------------------------------------------------------------------------
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



// -----------------------------------------------------------------------------
// LCD UPDATE
// -----------------------------------------------------------------------------
void updateLCD() {
  lcd.setCursor(0, 0);

  char buf[9];
  sprintf(buf, "%02d:%02d:%02d", hh, mm, ss);
  lcd.print(buf);
  lcd.print("   ");

  lcd.setCursor(0, 1);
  lcd.print("Dir:");
  lcd.print(dirCW ? "C " : "CC");
  lcd.print(" Spd:");
  lcd.print(speedLabel);
  lcd.print("   ");
}



// -----------------------------------------------------------------------------
// SOUND PEAK-TO-PEAK SAMPLING
// -----------------------------------------------------------------------------
int readSoundP2P() {
  unsigned long startTime = millis();
  int maxVal = 0;
  int minVal = 1023;

  while (millis() - startTime < 50) {
    int sample = analogRead(soundPin);
    if (sample > maxVal) maxVal = sample;
    if (sample < minVal) minVal = sample;
  }

  return (maxVal - minVal);
}
