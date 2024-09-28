// Pin definitions
const int buttonPins[] = {2, 3, 4, 5};  // Player button pins
const int ledPins[] = {6, 7, 8, 9};     // Player LED pins
const int resetButtonPin1 = 10;         // First reset button
const int resetButtonPin2 = 12;         // Second reset button
const int resetLEDPin = 11;             // Reset LED
const int standbyLEDPin = 13;           // Standby LED

// Constants
const long debounceDelay = 50;          // Debounce time for buttons (in ms)
const long blinkInterval = 300;         // Interval for blinking reset LED (in ms)

// Variables
int winner = -1;
bool gameEnded = false;
unsigned long previousMillis = 0;
unsigned long lastDebounceTimeReset1 = 0;  // Debounce timer for reset button 1
unsigned long lastDebounceTimeReset2 = 0;  // Debounce timer for reset button 2

void setup() {
  // Initialize button and LED pins
  for (int i = 0; i < 4; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
    pinMode(ledPins[i], OUTPUT);
  }

  pinMode(resetButtonPin1, INPUT_PULLUP);
  pinMode(resetButtonPin2, INPUT_PULLUP);
  pinMode(resetLEDPin, OUTPUT);
  pinMode(standbyLEDPin, OUTPUT);

  digitalWrite(standbyLEDPin, HIGH); // Turn on standby LED initially
}

void loop() {
  if (!gameEnded) {
    checkPlayerButtons();  // Continuously check for player button presses
  } else {
    handleGameEnd();       // Manage game end logic, including reset handling
  }
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

// Handle the game end state, including lighting LEDs and reset logic
void handleGameEnd() {
  for (int i = 0; i < 4; i++) {
    if (i == winner) {
      digitalWrite(ledPins[i], HIGH);  // Keep the winner's LED ON
    } else {
      digitalWrite(ledPins[i], LOW);   // Turn off other LEDs
      pinMode(buttonPins[i], INPUT);   // Disable all player buttons
    }
  }

  digitalWrite(standbyLEDPin, LOW);   // Turn off standby LED

  // Blink reset LED and check reset buttons
  blinkResetLED();
  if (checkResetButtons()) {
    resetGame();  // Reset the game if either reset button is pressed
  }
}

// Blink the reset LED at a set interval
void blinkResetLED() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= blinkInterval) {
    previousMillis = currentMillis;
    digitalWrite(resetLEDPin, !digitalRead(resetLEDPin));  // Toggle reset LED
  }
}

// Check if either reset button is pressed with debouncing
bool checkResetButtons() {
  unsigned long currentMillis = millis();

  // Debouncing for reset button 1
  if (digitalRead(resetButtonPin1) == LOW && (currentMillis - lastDebounceTimeReset1) > debounceDelay) {
    lastDebounceTimeReset1 = currentMillis;  // Update debounce time
    return true;  // Button 1 pressed, return true to reset
  } 

  // Debouncing for reset button 2
  if (digitalRead(resetButtonPin2) == LOW && (currentMillis - lastDebounceTimeReset2) > debounceDelay) {
    lastDebounceTimeReset2 = currentMillis;  // Update debounce time
    return true;  // Button 2 pressed, return true to reset
  }

  return false;  // Neither button pressed
}

// Reset the game state and turn off all LEDs
void resetGame() {
  // Turn off all player LEDs
  for (int i = 0; i < 4; i++) {
    digitalWrite(ledPins[i], LOW);
    pinMode(buttonPins[i], INPUT_PULLUP);  // Re-enable player buttons
  }

  // Reset game state
  gameEnded = false;
  winner = -1;
  digitalWrite(resetLEDPin, LOW);      // Turn off reset LED
  digitalWrite(standbyLEDPin, HIGH);   // Turn on standby LED
}
