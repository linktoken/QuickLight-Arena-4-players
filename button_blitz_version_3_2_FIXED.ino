// Code version 3.1.0 - FIXED
/*Change Log - Version 3.1.0
Release Date: April 2025

New Features & Changes:
✅ Added Siren Relay Trigger on A4
✅ Changed Winner LED Behavior
✅ Fixed foot switch logic (inverted condition)
✅ Fixed debounce array size issue
✅ Added proper serial debugging
✅ Improved code structure and comments

FIXES APPLIED:
- Fixed foot switch logic: Now correctly prevents button presses when foot switch IS pressed
- Fixed debounce array size: Changed from [3] to [4] to accommodate all inputs
- Added serial communication for debugging
- Improved variable naming and code clarity
*/

// Created by kncm_ken@hotmail.co.th (Kittipon N)
// Modified on April 2025 to add siren relay trigger output

// Pin definitions
const int buttonPins[] = {2, 3, 4, 5};  // Player button pins
const int ledPins[] = {6, 7, 8, 9};     // Player LED pins
const int resetButtonPins[] = {10, 12}; // Reset buttons
const int resetLEDPin = 11;             // Reset LED
const int standbyLEDPin = 13;           // Standby LED
const int footSwitchPin = A5;           // Foot switch moved to A5
const int sirenRelayPin = A4;           // Siren relay trigger output on A4

// Constants
const long debounceDelay = 50;          // Debounce time for buttons (in ms)
const int NUM_PLAYERS = 4;
const int NUM_RESET_BUTTONS = 2;

// Variables
int winner = -1;
bool gameEnded = false;
// Fixed: Changed array size from [3] to [4] to handle 2 reset buttons + 1 foot switch + 1 spare
unsigned long lastDebounceTimes[4] = {0, 0, 0, 0};  

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(9600);
  Serial.println("Game Controller v3.1.0 Starting...");
  
  // Initialize button and LED pins
  for (int i = 0; i < NUM_PLAYERS; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW); // Ensure all LEDs start OFF
  }

  // Initialize reset buttons
  for (int i = 0; i < NUM_RESET_BUTTONS; i++) {
    pinMode(resetButtonPins[i], INPUT_PULLUP);
  }

  // Initialize control pins
  pinMode(resetLEDPin, OUTPUT);
  pinMode(standbyLEDPin, OUTPUT);
  pinMode(footSwitchPin, INPUT_PULLUP);
  pinMode(sirenRelayPin, OUTPUT);
  
  // Set initial states
  digitalWrite(resetLEDPin, LOW);       // Reset LED OFF initially
  digitalWrite(standbyLEDPin, HIGH);    // Turn on standby LED initially
  digitalWrite(sirenRelayPin, LOW);     // Initialize siren relay as OFF
  
  Serial.println("Game ready - waiting for players...");
}

void loop() {
  if (!gameEnded) {
    // FIXED: Corrected foot switch logic
    // When foot switch is NOT pressed (HIGH due to INPUT_PULLUP), allow button presses
    if (digitalRead(footSwitchPin) == HIGH) {
      checkPlayerButtons();
    } else {
      // Foot switch is pressed - buttons are disabled
      // Optional: Add visual indication that buttons are disabled
    }
  } else {
    handleGameEnd();
  }

  // Always check reset inputs regardless of game state
  checkResetButtons();
}

// Function to check player buttons and detect winner
void checkPlayerButtons() {
  for (int i = 0; i < NUM_PLAYERS; i++) {
    if (digitalRead(buttonPins[i]) == LOW) {
      winner = i;
      gameEnded = true;
      
      // Turn ON winner LED steadily
      digitalWrite(ledPins[i], HIGH);
      
      // Activate siren relay when a player wins
      digitalWrite(sirenRelayPin, HIGH);
      
      // Turn off standby LED immediately
      digitalWrite(standbyLEDPin, LOW);
      
      // Turn on reset LED to indicate game over
      digitalWrite(resetLEDPin, HIGH);
      
      Serial.print("Player ");
      Serial.print(i + 1);
      Serial.println(" wins!");
      
      break;
    }
  }
}

// Handle the game end state
void handleGameEnd() {
  // Ensure winner LED stays ON and all others stay OFF
  for (int i = 0; i < NUM_PLAYERS; i++) {
    if (i == winner) {
      digitalWrite(ledPins[i], HIGH);   // Keep winner LED ON
    } else {
      digitalWrite(ledPins[i], LOW);    // Keep other LEDs OFF
    }
  }

  // Maintain game over state indicators
  digitalWrite(standbyLEDPin, LOW);     // Keep standby LED OFF
  digitalWrite(resetLEDPin, HIGH);      // Keep reset LED ON
  // Siren remains ON (no need to set it again)
}

// Check if any reset button or foot switch is pressed
void checkResetButtons() {
  unsigned long currentMillis = millis();

  // Check reset buttons
  for (int i = 0; i < NUM_RESET_BUTTONS; i++) {
    if (digitalRead(resetButtonPins[i]) == LOW && 
        (currentMillis - lastDebounceTimes[i]) > debounceDelay) {
      lastDebounceTimes[i] = currentMillis;
      Serial.print("Reset button ");
      Serial.print(i + 1);
      Serial.println(" pressed");
      resetGame();
      return; // Exit early after reset
    }
  }

  // Check foot switch as reset option (index 2 in debounce array)
  if (digitalRead(footSwitchPin) == LOW && 
      (currentMillis - lastDebounceTimes[2]) > debounceDelay) {
    lastDebounceTimes[2] = currentMillis;
    Serial.println("Foot switch reset activated");
    resetGame();
  }
}

// Reset the game state and turn off all LEDs
void resetGame() {
  Serial.println("Resetting game...");
  
  // Turn off all player LEDs
  for (int i = 0; i < NUM_PLAYERS; i++) {
    digitalWrite(ledPins[i], LOW);
    // Note: pinMode is already set in setup(), no need to reset it
  }

  // Reset game state variables
  gameEnded = false;
  winner = -1;
  
  // Set LED states for new game
  digitalWrite(resetLEDPin, LOW);       // Turn off reset LED
  digitalWrite(standbyLEDPin, HIGH);    // Turn on standby LED
  digitalWrite(sirenRelayPin, LOW);     // Turn off siren relay
  
  Serial.println("Game reset complete - ready for new game");
}
