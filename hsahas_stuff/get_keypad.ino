/* Imports */
#include <Keypad.h>

/* Keypad pins and map */
const byte ROWS = 4;
const byte COLS = 4;
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

/* LEDs / buzzer (fix the #defines) */
#define RED_LED    23
#define YELLOW_LED 25
#define GREEN_LED  27

/* Vars (init up front) */
const int yellow_time = 3;
int red_time   = 1;
int green_time = 1;

/* Simple interrupt flag (example: attach to a pin change or button) */
volatile bool keypad_request = false;
void IRAM_ATTR onKeypadInterrupt() {  // IRAM_ATTR only needed on ESP; harmless on AVR
  keypad_request = true;
}

/* Parse one command like A12#; returns true on success */
bool get_keypad(char &keypad_letter, int &timer_sec, unsigned long timeout_ms = 5000) {
  enum State { WANT_LETTER, WANT_D1, WANT_D2, WANT_HASH };
  State st = WANT_LETTER;

  int d1 = -1, d2 = -1;
  unsigned long t0 = millis();

  while (millis() - t0 < timeout_ms) {
    char k = customKeypad.getKey();
    if (!k) {
      // no key yet, keep waiting
      continue;
    }

    // Optional: allow '*' to cancel/reset state mid-entry
    if (k == '*') {
      st = WANT_LETTER; d1 = d2 = -1;
      continue;
    }

    switch (st) {
      case WANT_LETTER:
        // Accept A/B/C/D (or allow letters anywhere in map)
        if (k == 'A' || k == 'B' || k == 'C' || k == 'D') {
          keypad_letter = k;
          st = WANT_D1;
        } else {
          // Ignore anything that isn't the starting letter
        }
        break;

      case WANT_D1:
        if (k >= '0' && k <= '9') {
          d1 = k - '0';
          st = WANT_D2;
        } else {
          // Bad key: reset the sequence and look for a fresh letter
          st = WANT_LETTER; d1 = d2 = -1;
        }
        break;

      case WANT_D2:
        if (k >= '0' && k <= '9') {
          d2 = k - '0';
          st = WANT_HASH;
        } else {
          st = WANT_LETTER; d1 = d2 = -1;
        }
        break;

      case WANT_HASH:
        if (k == '#') {
          timer_sec = d1 * 10 + d2;
          Serial.print("Command received: ");
          Serial.print(keypad_letter);
          Serial.print(", ");
          Serial.println(timer_sec);
          return true;
        } else {
          // Anything other than '#' invalidates the sequence
          st = WANT_LETTER; d1 = d2 = -1;
        }
        break;
    }
  }

  // Timeout
  return false;
}

void setup() {
  Serial.begin(115200);

  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  // Example interrupt: change to your actual interrupt pin & mode
  // attachInterrupt(digitalPinToInterrupt(10), onKeypadInterrupt, FALLING);
}

void loop() {
  if (keypad_request) {
    noInterrupts();
    keypad_request = false;
    interrupts();

    char letter = '\0';
    int seconds = 0;
    if (get_keypad(letter, seconds, 7000)) {
      // Use the outputs (letter -> mode, seconds -> timer)
      // Example:
      // if (letter == 'A') red_time = seconds;
      // if (letter == 'B') yellow_time = seconds;  // yellow_time is const here; change if desired
      // if (letter == 'C') green_time = seconds;
    } else {
      Serial.println("Keypad read timed out or invalid sequence.");
    }
  }

  // ... rest of your state machine / blinking logic ...
}
