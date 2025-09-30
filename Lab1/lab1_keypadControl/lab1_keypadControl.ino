//File for keypad controls.
//Lib file for control found through search libraries, found out how to use online
/*From left to right while looking at keypad, pins on keypad are connected to pins 9->2

*/

#include <Keypad.h>

const byte ROWS = 4; // Four rows
const byte COLS = 4; // Four columns

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {9, 8, 7, 6}; // Connect to the row pinouts of the keypad (adjust as needed)
byte colPins[COLS] = {5, 4, 3, 2}; // Connect to the column pinouts of the keypad (adjust as needed)

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS);

void setup() {
  Serial.begin(9600); // Initialize serial communication for output

}

void loop() {
  char customKey = keypad.getKey(); // Check for key press

  if (customKey) { // If a key is pressed
    Serial.println(customKey); // Print the pressed key to the Serial Monitor
  }

}
