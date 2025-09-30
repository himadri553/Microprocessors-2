/*
  MVP of 7 segment display counting down remaining time
  - Set up working 1 sec timer
  - Total time can be configured

*/

/*Imports*/
#include <avr/interrupt.h>

/*Pins*/
int latch=9;  
int clock_pin=10; 
int data=8;   

/*Vars*/
// Hex map for display
unsigned char table[]=
{0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x77,0x7c
,0x39,0x5e,0x79,0x71,0x00};

int timer_count = 9;  // Default starting timer value
int user_time = 9;

/*Helper functions*/
void Display(unsigned char num)
{
  digitalWrite(latch,LOW);
  shiftOut(data,clock_pin,MSBFIRST,table[num]);
  digitalWrite(latch,HIGH);
}

// ISR triggred at each second
ISR(TIMER1_COMPA_vect) {
  timer_count--;
  if (timer_count < 0) {
    timer_count = user_time;
  }
}

/*To run*/
void setup() {
  Serial.begin(9600);

  // Setup display
  pinMode(latch,OUTPUT);
  pinMode(clock_pin,OUTPUT);
  pinMode(data,OUTPUT);

  // Setup timer 1 - CTC mode
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  OCR1A = 15624;                       // 1 sec at 16MHz/1024
  TCCR1B |= (1 << WGM12);              // CTC mode
  TCCR1B |= (1 << CS12) | (1 << CS10); // Prescaler 1024
  TIMSK1 |= (1 << OCIE1A);             // Enable Timer1 compare interrupt

  // Enable global interupts
  sei();

}

void loop() {
  if (Serial.available() > 0) {
    String user_input = Serial.readStringUntil('\n');         
    user_time = user_input.toInt();   
  }

  Display(timer_count);

}

