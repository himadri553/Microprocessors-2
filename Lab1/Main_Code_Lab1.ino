/*
  Lab 1: Traffic Light Controller
  EECE.5520 - Microprocessor II and Embedded System Design

  Himadri Saha, Dan Burns, Chris Worthley

*/

#include <stdio.h>
#include <stdlib.h>
//#include "lab1_keypadControl.ino"
//#include assembly light code

//pins for LEDs
int green_main=22;  // GP0 pin Green
int yellow_main=23; // GP1 pin Red
int red_main=24; //GP2 pin Yellow
int green_cross=25;  // GP0 pin Green
int yellow_cross=26; // GP1 pin Red
int red_cross=27; //GP2 pin Yellow

//global vars for redlight and greenlight times
int main_time_greenlight=10;
int main_time_redlight=10;

unsigned long start_time=0;
unsigned long current_time=0;



void setup() {
      Serial.begin(9600); // Initialize serial communication at 9600 baud
    }


//Function, in assembly, to set the LEDs (NEEDS TO BE DONE)
void set_led_assembly(int led){

}




int test_func_leds(){ //Test function to use without assembly code, and without keypad to run LEDs. 
//Take this code, paste to main, and write set_led_assembly. Also integrate with keypad and 7 segment display
//ALSO NEED TO INTEGRATE BUZZER
    //Set LED pins to output
    pinMode(22, OUTPUT);
    pinMode(23, OUTPUT);
    pinMode(24, OUTPUT);
    pinMode(25, OUTPUT);
    pinMode(26, OUTPUT);
    pinMode(27, OUTPUT);
    /*
        INSERT KEYPAD AND DISPLAY CODE HERE
    */

    //main loop. will loop lights after times are confirmed
    while(true){
        //STATE 1 RUNNING STATE, main redlight on, cross greenlight on
        digitalWrite(red_main, HIGH); //turn on main redlight
        digitalWrite(green_cross, HIGH); //turn on cross greenlight
        start_time=millis(); //get start time offset
        while((main_time_redlight-((current_time-start_time))/1000)>6){ //loop until 6 seconds left in main redlight time
            current_time=millis();
        }
        //STATE 2 RUNNING STATE, main redlight on, cross greenlight blinking
        for(int i=0;i<3;i++){ //blink the cross green light for 3 secs then turn it off. Main redlight stays ON. one loop is 1 sec 
            digitalWrite(green_cross,HIGH); //turn on light
            delay(500);//wait 500ms (0.5s)
            digitalWrite(green_cross,LOW); //turn off light
            delay(500);//wait 500ms
        }
        //STATE 3 RUNNING STATE, main redlight blinking, cross yellowlight on
        digitalWrite(yellow_cross,HIGH);//turn on cross yellow light
        for(int i=0;i<3;i++){ //blink main redlight for 3 secs then turn off
            digitalWrite(red_main,HIGH); //turn on light
            delay(500);//wait 500ms (0.5s)
            digitalWrite(red_main,LOW); //turn off light
            delay(500);//wait 500ms
        }
        digitalWrite(yellow_cross,LOW);//turn off cross yellowlight
        //STATE 4 RUNNING STATE, main greenlight on, cross redlight is on
        digitalWrite(green_main, HIGH); //turn on main greenlight
        digitalWrite(red_cross, HIGH); //turn on cross redlight
        start_time=millis(); //get start time offset
        current_time=millis();
        while((main_time_greenlight-((current_time-start_time)/1000))>3){ //loop until 3 seconds left in main greenlight time
            current_time=millis(); //get current time
        }
        //STATE 5 RUNNING STATE, main greenlight blinking, cross redlight on
        for(int i=0;i<3;i++){ //blink main redlight for 3 secs then turn off
            digitalWrite(green_main,HIGH); //turn on light
            delay(500);//wait 500ms (0.5s)
            digitalWrite(green_main,LOW); //turn off light
            delay(500);//wait 500ms
        }

        //STATE 6 RUNNING STATE, main yellowlight on, cross redlight blinking
        digitalWrite(yellow_main,HIGH);
        for(int i=0;i<3;i++){ //blink main redlight for 3 secs then turn off
            digitalWrite(red_cross,HIGH); //turn on light
            delay(500);//wait 500ms (0.5s)
            digitalWrite(red_cross,LOW); //turn off light
            delay(500);//wait 500ms
        }
        digitalWrite(yellow_main,LOW);//turn off cross yellowlight

        //LOOPS BACK TO STATE 1 RUNNING STATE...^
    }

}


 void loop(){
    Serial.begin(9600); // Initialize serial communication at 9600 baud
    delay(1000);
    test_func_leds();


 }


