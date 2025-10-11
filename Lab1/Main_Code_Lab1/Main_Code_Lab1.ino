/*
  Lab 1: Traffic Light Controller
  EECE.5520 - Microprocessor II and Embedded System Design

  Himadri Saha, Dan Burns, Chris Worthley

  PIN SETUP
  Keypad: (going from 1-8) 12 -> 5
  Traffic signal 1 (green, yellow, red): 52, 50, 48
  Traffic signal 2: 53, 51, 49
  Buzzer: PWM 13
  7 segment display (by wires):
    Clock (green): 4
    Data (orange): 3
    Latch (yellow): 2
  
  TODO:
  Project description said to use timer interupts to advance state machine??? 
  - There would be a timer (like a heartbeat) running (and tracking?) every second
  - used to advance state machine 
  - would check for keypad press and act accordingly
  - current time could be refrenced using the heartbeat for led timing and displaying 

*/

/* Imports */
#include <stdio.h>
#include <stdlib.h>
#include <Keypad.h>
#include <avr/interrupt.h>

/* Pins assignments (using #define to get more RAM space) */
#define  green_main 52     
#define  yellow_main 50    
#define  red_main 48        
#define  green_cross 53    
#define  yellow_cross 51
#define  red_cross 49 
#define  buzzer_pin 13
#define  clock_pin 4
#define  data_pin 3
#define  latch_pin 2

/* Init Vars */
 // timing and state transistion stuff
volatile bool tick_flag = false;   
int countdown_timer = 0;
int state = 1;             
// Keypad setup (from elegoo setup code) 
const byte ROWS = 4; 
const byte COLS = 4; 
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {12, 11, 10, 9};
byte colPins[COLS] = {8, 7, 6, 5}; 
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
// Hex map for display
unsigned char table[]=
{0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c
,0x39,0x5e,0x79,0x71,0x00};

// global vars for redlight and greenlight times
int main_time_greenlight=10;
int main_time_redlight=10;
unsigned long start_time=0;
unsigned long current_time=0;

// Keypad user input
String inputBuffer = "";

/* Helper functions */
// ISR to signal main loop that 1s passed
ISR(TIMER1_COMPA_vect) {
  tick_flag = true; 
}

/* needs to be checked... */
void update_leds(int s) {
  // Reset all lights first
  digitalWrite(red_main, LOW);
  digitalWrite(yellow_main, LOW);
  digitalWrite(green_main, LOW);
  digitalWrite(red_cross, LOW);
  digitalWrite(yellow_cross, LOW);
  digitalWrite(green_cross, LOW);

  // For blinking, we’ll use millis() to toggle every 500ms
  static unsigned long lastBlink = 0;
  static bool blinkOn = false;

  if (millis() - lastBlink >= 500) {
    blinkOn = !blinkOn;
    lastBlink = millis();
  }

  switch (s) {
    case 1: // Main RED on, Cross GREEN on (steady)
      digitalWrite(red_main, HIGH);
      digitalWrite(green_cross, HIGH);
      break;

    case 2: // Main RED steady, Cross GREEN blinking
      digitalWrite(red_main, HIGH);
      if (blinkOn) digitalWrite(green_cross, HIGH);
      break;

    case 3: // Main RED blinking, Cross YELLOW steady
      digitalWrite(yellow_cross, HIGH);
      if (blinkOn) digitalWrite(red_main, HIGH);
      break;

    case 4: // Main GREEN steady, Cross RED steady
      digitalWrite(green_main, HIGH);
      digitalWrite(red_cross, HIGH);
      break;

    case 5: // Main GREEN blinking, Cross RED steady
      digitalWrite(red_cross, HIGH);
      if (blinkOn) digitalWrite(green_main, HIGH);
      break;

    case 6: // Main YELLOW steady, Cross RED blinking
      digitalWrite(yellow_main, HIGH);
      if (blinkOn) digitalWrite(red_cross, HIGH);
      break;
  }
}

void update_display(unsigned char seconds_left) {
  digitalWrite(latch_pin,LOW);
  shiftOut(data_pin,clock_pin,MSBFIRST,table[seconds_left]);
  digitalWrite(latch_pin,HIGH);
}

void read_keypad() {
  char userKey = keypad.getKey();
  if (userKey) {
    // Output command
    if (userKey == '#') {   
      Serial.print("Command received: ");
      Serial.println(inputBuffer);
      inputBuffer = "";

      // reasign new timeing var here

    }
    // Add keypresses to command
    else {                    
      inputBuffer += userKey;
    }
  }
}

/* Main runner functions */
void setup() {
  // Pin and serial setup
  Serial.begin(9600);
  pinMode(green_main,OUTPUT);
  pinMode(yellow_main,OUTPUT);
  pinMode(red_main,OUTPUT);
  pinMode(green_cross,OUTPUT);
  pinMode(yellow_cross,OUTPUT);
  pinMode(red_cross,OUTPUT);
  pinMode(buzzer_pin,OUTPUT);
  pinMode(latch_pin,OUTPUT);
  pinMode(clock_pin,OUTPUT);
  pinMode(data_pin,OUTPUT);

  /*
    Setup timer - CTC mode
    1 sec at 16MHz/1024
    Prescaler 1024
    Enable Timer1 compare interrupt
  */ 
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  OCR1A = 15624;                       
  TCCR1B |= (1 << WGM12);              
  TCCR1B |= (1 << CS12) | (1 << CS10); 
  TIMSK1 |= (1 << OCIE1A);              
  sei();

  // Initalize state machine
  state = 1;
  countdown_timer = main_time_redlight;

}

void loop(){  
  /* Check and get keypad input at start of loop (change so that it changes the times it runs as well)*/ 
  read_keypad();



  /* state machine - update led based on state  */
  if (tick_flag) {
    tick_flag = false;
    countdown_timer--;

    // Only advance state when countdown_timer hits 0
    if (countdown_timer <= 0) {
      switch (state) {
        case 1:
          state = 2;
          countdown_timer = 3;
          Serial.println("state 1 -> 2");
          break;
        case 2:
          state = 3;
          countdown_timer = 3;
          Serial.println("state 2 -> 3");
          break;
        case 3:
          state = 4;
          countdown_timer = main_time_greenlight - 3;
          Serial.println("state 3 -> 4");
          break;
        case 4:
          state = 5;
          countdown_timer = 3;
          Serial.println("state 4 -> 5");
          break;
        case 5:
          state = 6;
          countdown_timer = 3;
          Serial.println("state 5 -> 6");
          break;
        case 6:
          state = 1;
          countdown_timer = main_time_redlight - 6;
          Serial.println("state 6 -> 1");
          break;
      }
    }
  
    update_leds(state);
  }
  
  // update display
  update_display(3);

  // Buzzer beep at 3 mark
  if (countdown_timer <= 3 && countdown_timer > 0) {
    digitalWrite(buzzer_pin, HIGH);
  } else {
    digitalWrite(buzzer_pin, LOW);
  }

  delay(100);
}