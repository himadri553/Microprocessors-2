/*
  Lab 1: Traffic Light Controller
  EECE.5520 - Microprocessor II and Embedded System Design

  Himadri Saha, Daniel Burns, Chris Worthley

  PIN SETUP
  Keypad: (going from 1-8) 12 -> 5
  Traffic signal 1 (green, yellow, red): 52, 50, 48
  Traffic signal 2: 53, 51, 49
  Buzzer: PWM 13
  7 segment display (by wires):
    Clock (green): 4
    Latch (orange): 3
    Data (yellow): 2
  
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
#define  latch_pin 3
#define  data_pin 2

/* Init Vars */
// timing and state transistion stuff  
bool phase1_flag = false;
int state;  
int display_time;           
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

// Get assembly function
extern "C" void set_red_leds_high();

/* Helper functions */
void update_display(unsigned char seconds_left) {
  digitalWrite(latch_pin,LOW);
  shiftOut(data_pin,clock_pin,MSBFIRST,table[seconds_left]);
  digitalWrite(latch_pin,HIGH);
}

ISR(TIMER1_COMPA_vect) {
  if (display_time < 0) {
    update_display(0);
  }
  else if (display_time > 8){
    update_display(8);
  }
  else {
    update_display(display_time);
  }

  display_time--;
}

String read_keypad() {
  char userKey;
  while (true) {
    userKey = keypad.getKey();
    if (userKey) {
      return String(userKey);
    }
  }
}

void beep() {
  digitalWrite(buzzer_pin, HIGH);
  delay(100);
  digitalWrite(buzzer_pin, LOW);
}

/* Main runner functions */
void setup() {
  // Pin and serial setup
  Serial.begin(9600);
  Serial.println();
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

  // Setup Timer1 for 1Hz
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 15624;                        // 16 MHz / 1024 / 1 Hz
  TCCR1B |= (1 << WGM12);              // CTC mode
  TCCR1B |= (1 << CS12) | (1 << CS10); // Prescaler 1024
  TIMSK1 |= (1 << OCIE1A);             // Enable compare match interrupt
  sei();

  // Initalize state machine
  state = 0;
  phase1_flag = true;
  update_display(0);
}

void loop(){  
  /* Phase 1 - Use keypad to set times */ 
  if (phase1_flag) {

    // Wait for "*" to start input phase with blinking leds
    Serial.println("Waiting for * to begin input...");
    while (true) {
      set_red_leds_high();
      String key = read_keypad();
      if (key == "*") {
        Serial.println("Begin entering command:");
        digitalWrite(red_main, LOW);
        digitalWrite(red_cross, LOW);
        break;
      }
    }

    // Now gather full command (expecting [letter][digit][digit][#])
    String inputBuffer = "";
    while (true) {
      String key = read_keypad();
      inputBuffer += key;

      if (key == "#") {
        // Got complete input — validate format
        if (inputBuffer.length() == 4 &&
            isAlpha(inputBuffer[0]) &&
            isDigit(inputBuffer[1]) &&
            isDigit(inputBuffer[2]) &&
            inputBuffer[3] == '#') {
            
          Serial.print("Valid command: ");
          Serial.println(inputBuffer);

          // extract and apply values
          char letter = inputBuffer[0];
          int num1 = inputBuffer[1] - '0';
          int num2 = inputBuffer[2] - '0';
          int full_value = num1 * 10 + num2;
          if (letter == 'A') main_time_redlight = full_value;
          if (letter == 'B') main_time_greenlight = full_value;
          Serial.println(main_time_redlight);
          Serial.println(main_time_greenlight);

          break;
        } else {
          Serial.print("Invalid format: ");
          Serial.println(inputBuffer);
          inputBuffer = "";  // Reset for new input
        }
      }
    }
  }

  /* Phase 2 - Cycle thru LEDs */
  while (true) {
    switch (state){
      case 0:
        /* STATE 1 RUNNING STATE, main redlight on, cross greenlight on */
        display_time = main_time_redlight;
        digitalWrite(red_main, HIGH);     //turn on main redlight
        digitalWrite(green_cross, HIGH);  //turn on cross greenlight
        start_time=millis();              //get start time offset              
        while((main_time_redlight-((current_time-start_time))/1000)>6){ 
          current_time=millis();
        }

        beep();
        state = 1;
        break;
      
      case 1:
        /* STATE 2 RUNNING STATE, main redlight on, cross greenlight blinking */
        for(int i=0;i<3;i++){             //blink the cross green light for 3 secs then turn it off. Main redlight stays ON. one loop is 1 sec 
          digitalWrite(green_cross,HIGH); //turn on light
          delay(500);                     //wait 500ms (0.5s)
          digitalWrite(green_cross,LOW);  //turn off light
          delay(500);                     //wait 500ms
        }

        state = 2;
        break;
      
      case 2:
        /* STATE 3 RUNNING STATE, main redlight blinking, cross yellowlight on */
        digitalWrite(yellow_cross,HIGH);    //turn on cross yellow light
        for(int i=0;i<3;i++){               //blink main redlight for 3 secs then turn off
          digitalWrite(red_main,HIGH);      //turn on light
          delay(500);                       //wait 500ms (0.5s)
          digitalWrite(red_main,LOW);       //turn off light
          delay(500);                       //wait 500ms
        }
        digitalWrite(yellow_cross,LOW);     //turn off cross yellowlight

        state = 3;
        break;

      case 3:
        /* STATE 4 RUNNING STATE, main greenlight on, cross redlight is on */
        digitalWrite(green_main, HIGH);   //turn on main greenlight
        digitalWrite(red_cross, HIGH);    //turn on cross redlight
        start_time=millis();              //get start time offset
        current_time=millis();
        //loop until 3 seconds left in main greenlight time
        while((main_time_greenlight - ((current_time-start_time)/1000)) > 3) {
          current_time=millis();          //get current time
        }

        beep();
        state = 4;
        break;

      case 4:
        /* STATE 5 RUNNING STATE, main greenlight blinking, cross redlight on */
        for(int i=0;i<3;i++){             //blink main redlight for 3 secs then turn off
          digitalWrite(green_main,HIGH);  //turn on light
          delay(500);                     //wait 500ms (0.5s)
          digitalWrite(green_main,LOW);   //turn off light
          delay(500);                     //wait 500ms
        }

        state = 5;
        break;

      case 5:
        /* STATE 6 RUNNING STATE, main yellowlight on, cross redlight blinking */
        digitalWrite(yellow_main,HIGH);
        for(int i=0;i<3;i++){             //blink main redlight for 3 secs then turn off
          digitalWrite(red_cross,HIGH);   //turn on light
          delay(500);                     //wait 500ms (0.5s)
          digitalWrite(red_cross,LOW);    //turn off light
          delay(500);                     //wait 500ms
        }
        digitalWrite(yellow_main,LOW);    //turn off cross yellowlight

        state = 0;
        break;
    }
  }
  
  delay(100);
}