// Code version 3.0.0
/*Change Log - Version 3.0
Release Date: March 2025

New Features & Changes:
✅ Foot Switch Moved to A5

Previously on A1, now correctly assigned to A5 for better wiring and functionality.
✅ Foot Switch Prevents Player Button Presses

While the foot switch is held, all player buttons are disabled to ensure proper control.
✅ Manual Reset Only (No Auto-Reset)

After a player wins, their LED blinks indefinitely until the game is manually reset.
Game only resets when a reset button or the foot switch (A5) is pressed.
✅ Improved Reset Handling

Added debounce protection for reset buttons and foot switch to prevent accidental resets.
Reset LED stays ON until the game is reset.
✅ Optimized Code Structure

Cleaned up loop logic for better efficiency.
Improved function readability and structure.
*/

// Created by kncm_ken@hotmail.co.th (Kittipon N)

// Pin definitions
const int buttonPins[] = {2, 3, 4, 5};  // Player button pins
const int ledPins[] = {6, 7, 8, 9};     // Player LED pins
const int resetButtonPins[] = {10, 12}; // Reset buttons
const int resetLEDPin = 11;             // Reset LED
const int standbyLEDPin = 13;           // Standby LED
const int footSwitchPin = A5;           // Foot switch moved to A5

// Constants
const long debounceDelay = 50;          // Debounce time for buttons (in ms)
const long blinkInterval = 300;         // Interval for blinking winner LED (in ms)

// Variables
int winner = -1;
bool gameEnded = false;
unsigned long previousMillis = 0;
unsigned long lastDebounceTimes[3] = {0, 0, 0};  // Debounce timers for reset buttons and foot switch

void setup() {
  // Initialize button and LED pins
  for (int i = 0; i < 4; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
    pinMode(ledPins[i], OUTPUT);
  }

  for (int i = 0; i < 2; i++) {
    pinMode(resetButtonPins[i], INPUT_PULLUP);
  }

  pinMode(resetLEDPin, OUTPUT);
  pinMode(standbyLEDPin, OUTPUT);
  pinMode(footSwitchPin, INPUT_PULLUP); // Ensure foot switch is a digital input with pull-up

  digitalWrite(standbyLEDPin, HIGH); // Turn on standby LED initially
}

void loop() {
  if (!gameEnded) {
    if (digitalRead(footSwitchPin) == HIGH) { // Only allow button presses if foot switch is NOT pressed
      checkPlayerButtons();
    }
  } else {
    handleGameEnd();
  }

  // Check reset inputs
  checkResetButtons();
}

// Function to check player buttons and detect winner
void checkPlayerButtons() {
  for (int i = 0; i < 4; i++) {
    if (digitalRead(buttonPins[i]) == LOW) {
      winner = i;               // Player i is the winner
      gameEnded = true;
      break;
    }
  }
}

// Handle the game end state (Blink winner LED but do not reset automatically)
void handleGameEnd() {
  unsigned long currentMillis = millis();

  for (int i = 0; i < 4; i++) {
    if (i == winner) {
      if (currentMillis - previousMillis >= blinkInterval) {
        previousMillis = currentMillis;
        digitalWrite(ledPins[i], !digitalRead(ledPins[i]));  // Blink winner LED
      }
    } else {
      digitalWrite(ledPins[i], LOW);   // Keep other LEDs OFF
    }
  }

  digitalWrite(standbyLEDPin, LOW);   // Turn off standby LED
  digitalWrite(resetLEDPin, HIGH);    // Keep reset LED ON (indicating game over)
}

// Check if any reset button or foot switch is pressed (but no auto-reset)
void checkResetButtons() {
  unsigned long currentMillis = millis();

  for (int i = 0; i < 2; i++) {
    if (digitalRead(resetButtonPins[i]) == LOW && (currentMillis - lastDebounceTimes[i]) > debounceDelay) {
      lastDebounceTimes[i] = currentMillis;
      resetGame();  // Reset the game only when the reset button is pressed
    }
  }

  // Foot switch as an additional reset option
  if (digitalRead(footSwitchPin) == LOW && (currentMillis - lastDebounceTimes[2]) > debounceDelay) {
    lastDebounceTimes[2] = currentMillis;
    resetGame();
  }
}

// Reset the game state and turn off all LEDs
void resetGame() {
  for (int i = 0; i < 4; i++) {
    digitalWrite(ledPins[i], LOW);
    pinMode(buttonPins[i], INPUT_PULLUP);
  }

  gameEnded = false;
  winner = -1;
  digitalWrite(resetLEDPin, LOW);
  digitalWrite(standbyLEDPin, HIGH);
}
