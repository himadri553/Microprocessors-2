/*
  Micro Lab 1

  Classes/Funcitions:
  - Get command from key pad
  - Get some time from a timer -> letter_for_LED, durations (from 2 digits)
  - Start operation of traffic lights (int duration from keypad)

  Higher Level Design:
  - Inital start (red light blinking)
  - 

  Inital State
  Operational State (from get keypad)


*/

/* Imports */
#include <Keypad.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

/* Pins  */
// Keypad setup - from Elegoo Keypad Module docs
const byte ROWS = 4; 
const byte COLS = 4; 
char hexaKeys[ROWS][COLS] = {
{'1','2','3','A'},
 {'4','5','6','B'},
 {'7','8','9','C'},
 {'*','0','#','D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {5, 4, 3, 2}; //connect to the column pinouts of the keypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS,
COLS); 

// LED and Buzzer Pins
#define RED_LED = 23;
#define YELLOW_LED = 25;
#define GREEN_LED = 27;
#define BUZZER = 12;

/* Vars */
#define yellow_time = 3;
int red_time = 1;
int green_time = 1;

// State flags
bool user_input = false; // changes at keypad interupt

/* Helper functions */
void ISR (INT2_vect) {
  user_input = true;
}

void get_keypad_command(){
  /* With an interupt, return info for operational_state */

  return keypad_letter, timer_sec
}

/* Diffrent states of the traffic light */
void startup_mode() {
  /* Blinks LED before keypad comes in with a command (req 1) */
  
  return 
}

void set_timer(char keypad_letter, int timer_sec) {
  /* After getting keypad command, set timer consts and flash lights accordingly (req 2) */

  // Set LED

  // Run standard operation

  return
}

void operational_mode() {
  /* After "*", go to operatonal mode based on  (req 3) */

}

/* Ardunio Functions */
void setup() {
  Serial.begin(115200);
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  // Attach interupt to pin 2 (letter col)
  pinMode(INT_PIN, INPUT_PULLUP); // button to GND, so idle=HIGH, press=LOW
  attachInterrupt(digitalPinToInterrupt(INT_PIN), onInterrupt, FALLING);

}

void loop() {
  // Check if user pressed key pad
  if (user_input) {

  }
  
  else {

  }

  // Reset user input flag back to false after
  user_input = false;

}
