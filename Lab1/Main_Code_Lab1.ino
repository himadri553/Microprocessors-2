#include <stdio.h>
#include <stdlib.h>
#include "lab1_keypadControl.ino"
//#include assembly light code


#define green 0  // GP0 pin Green
#define red 1 // GP1 pin Red
#define yellow 2 //GP2 pin Yellow
 
int set_red_time(char command[]){
    char[] time;
    strcpy(command, time, 1, 2);
    return int red_time=atoi(time);
}

int set_green_time(char command[]){
    char[] time;
    strcpy(command, time, 1, 2);
    return int green_time=atoi(time);
}

void run_red(){
    //calls assembly code to light red LED

}

void kill_red(){
    //calls assembly code to kill red LED

}

void run_green(){
    //calls assembly code to light green LED
}

void kill_green(){
    //calls assembly code to kill green LED

}

void run_yellow(){
    //calls assembly code to light yellow LED

}

void kill_yellow(){
    //calls assembly code to kill yellow LED

}


void run_traffic_light() {
    gpio_init(green);               // Initialize GP0
    gpio_set_dir(green, GPIO_OUT);   // Set GP0 as an output
    gpio_init(red);               // Initialize GP1
    gpio_set_dir(red, GPIO_OUT);   // Set GP1 as an output             
    gpio_init(yellow);               // Initialize GP2
    gpio_set_dir(yellow, GPIO_OUT);   // Set GP2 as an output  

    int red=-1;
    int green=-1;
    char last;
    char exit[4]='';

    while(command[0]=='*'){
        command[]='';
        while(command[count]=='#'){    //checks if # is pressed to terminate the keypad process
        //Takes in the key presses an an string
        count++;
        char command[]=/*string from keypad*/;
        color=command[0];
            if(command[count]=='*'){
                last=command[0];
                command[]='*';
                break;
            }
        }

        switch (keys){
            case color=='A' && command != '*':
                red=set_red_time(command);
                break;
            case color=='B' && command != '*':
                green=set_green_time(command);
                break;
        }
    }
    
    if(last=='A'){
       while(exit=='exit'){ //the exit command is for testing
            if(red==-1){
                printf("Error Red wasn't assigned");
                break;
            }
            run_red();
            sleep_ms((red-3)*1000);
            for(int i=0; i<3; i++){
                run_red();
                sleep_ms(500);
                kill_red();
                sleep_ms(500);
            }

            if(green=-1){
                printf("Error Green wasn't assigned")
                break;
            }
            run_green();
            sleep_ms((green-3)*1000);
            for(int i=0; i<3; i++){
                run_green();
                sleep_ms(500);
                kill_green();
                sleep_ms(500);
            }

            run_yellow();
            sleep_ms(3000);
            kill_yellow();
       }

    }else if(last=='B'){
        while(exit=='exit'){ //the exit command is for testing
            if(green==-1){
                printf("Error Green wasn't assigned");
                break;
            }
            run_green();
            sleep_ms((green-3)*1000);
            for(int i=0; i<3; i++){
                run_green();
                sleep_ms(500);
                kill_green();
                sleep_ms(500);
            }

            run_yellow();
            sleep_ms(3000);
            kill_yellow();

            if(red=-1){
                printf("Error Red wasn't assigned")
                break;
            }
            run_red();
            sleep_ms((red-3)*1000);
            for(int i=0; i<3; i++){
                run_red();
                sleep_ms(500);
                kill_red();
                sleep_ms(500);
            }
       }
    }
    printf("Done");
}
