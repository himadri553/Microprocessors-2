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

int test_func_leds(){ 
  /*
  Test function to use without assembly code, and without keypad to run LEDs. 
  Take this code, paste to main, and write set_led_assembly. Also integrate with keypad and 7 segment display
  ALSO NEED TO INTEGRATE BUZZER
  */
  /* 
    Start loop by checking for user keypad input 
    (is getting missed, user should be able to input at any point, while light sequence is still running)
  */
  char userKey = keypad.getKey();
  if (userKey) {
    if (userKey == '#') {     // Output command
      Serial.print("Command received: ");
      Serial.println(inputBuffer);
      inputBuffer = "";
    }
    else {
      inputBuffer += userKey; // Add keypresses to command
    }
  }

  //main loop. will loop lights after times are confirmed
  while(true){
    /* STATE 1 RUNNING STATE, main redlight on, cross greenlight on */
    digitalWrite(red_main, HIGH);     //turn on main redlight
    digitalWrite(green_cross, HIGH);  //turn on cross greenlight
    start_time=millis();              //get start time offset
    //loop until 6 seconds left in main redlight time
    while((main_time_redlight-((current_time-start_time))/1000)>6){ 
      current_time=millis();
    }
      
    /* STATE 2 RUNNING STATE, main redlight on, cross greenlight blinking */
    for(int i=0;i<3;i++){             //blink the cross green light for 3 secs then turn it off. Main redlight stays ON. one loop is 1 sec 
      digitalWrite(green_cross,HIGH); //turn on light
      delay(500);                     //wait 500ms (0.5s)
      digitalWrite(green_cross,LOW);  //turn off light
      delay(500);                     //wait 500ms
    }
      
    /* STATE 3 RUNNING STATE, main redlight blinking, cross yellowlight on */
    digitalWrite(yellow_cross,HIGH);    //turn on cross yellow light
    for(int i=0;i<3;i++){               //blink main redlight for 3 secs then turn off
      digitalWrite(red_main,HIGH);      //turn on light
      delay(500);                       //wait 500ms (0.5s)
      digitalWrite(red_main,LOW);       //turn off light
      delay(500);                       //wait 500ms
    }
    digitalWrite(yellow_cross,LOW);     //turn off cross yellowlight
        
    /* STATE 4 RUNNING STATE, main greenlight on, cross redlight is on */
    digitalWrite(green_main, HIGH);   //turn on main greenlight
    digitalWrite(red_cross, HIGH);    //turn on cross redlight
    start_time=millis();              //get start time offset
    current_time=millis();
    //loop until 3 seconds left in main greenlight time
    while((main_time_greenlight-((current_time-start_time)/1000))>3){ 
      current_time=millis();          //get current time
    }
        
    /* STATE 5 RUNNING STATE, main greenlight blinking, cross redlight on */
    for(int i=0;i<3;i++){             //blink main redlight for 3 secs then turn off
      digitalWrite(green_main,HIGH);  //turn on light
      delay(500);                     //wait 500ms (0.5s)
      digitalWrite(green_main,LOW);   //turn off light
      delay(500);                     //wait 500ms
    }

    /* STATE 6 RUNNING STATE, main yellowlight on, cross redlight blinking */
    digitalWrite(yellow_main,HIGH);
    for(int i=0;i<3;i++){             //blink main redlight for 3 secs then turn off
      digitalWrite(red_cross,HIGH);   //turn on light
      delay(500);                     //wait 500ms (0.5s)
      digitalWrite(red_cross,LOW);    //turn off light
      delay(500);                     //wait 500ms
    }
    digitalWrite(yellow_main,LOW);    //turn off cross yellowlight

    //LOOPS BACK TO STATE 1 RUNNING STATE...^
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
  Serial.begin(9600); 

  // for testing: test_func_leds();
  
  /* Check and get keypad input at start of loop (change so that it changes the times it runs as well)*/ 
  char userKey = keypad.getKey();
  if (userKey) {
    // Output command
    if (userKey == '#') {   
      Serial.print("Command received: ");
      Serial.println(inputBuffer);
      inputBuffer = "";
    }
    // Add keypresses to command
    else {                    
      inputBuffer += userKey;
    }
  }

  /* Advance state machine only when ISR heartbeat says to */   
  if (tick_flag) { // runs when "heart beats"
    
    // reset tick flag and decrement timer
    tick_flag = false;
    countdown_timer--;

    // state machine
    switch (state) {
      case 1:
        state = 2;
        countdown_timer = 3;
        Serial.print("main redlight on, cross greenlight on");
        break;

      case 2:
        state = 3;
        countdown_timer = 3;
        Serial.print("main redlight on, cross greenlight blinking");
        break;

      case 3:
        state = 4;
        countdown_timer = main_time_greenlight - 3;
        Serial.print("main redlight blinking, cross yellowlight on ");
        break;

      case 4:
        state = 5;
        countdown_timer = 3;
        Serial.print("main greenlight on, cross redlight is on");
        break;

      case 5:
        state = 6;
        countdown_timer = 3;
        Serial.print("main greenlight blinking, cross redlight on");
        break;

      case 6:
        state = 1;
        countdown_timer = main_time_redlight - 6;
        Serial.print("main yellowlight on, cross redlight blinking");
        break;

    }
  }

  /* Update outputs (led, buzzer and display) */
  // update led based on state
  update_leds(state);

  // Buzzer beep at 3 mark
  if (countdown_timer <= 3 && countdown_timer > 0) {
    digitalWrite(buzzer_pin, HIGH);
  } else {
    digitalWrite(buzzer_pin, LOW);
  }

  // update display
  update_display(countdown_timer);
}


