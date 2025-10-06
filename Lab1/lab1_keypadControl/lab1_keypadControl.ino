/*
  Keypad control MVP
  
  Will take in a command (at anytime) in the form of [letter][number][number][hashtag] and output it on the serial monitor at the begining of the loop
  If the hashtag is not pressed, then it will not be valid

  Pin setup:
  Keypad pins (1-8) are connected on ardunio pins (9-2)

*/
#include <Keypad.h>

// Keypad setup (from elegoo setup code)
const byte ROWS = 4; 
const byte COLS = 4; 
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String inputBuffer = "";

void setup() {
  Serial.begin(9600);

}

void loop() {
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
}
