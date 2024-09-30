// Code version 2.1
// Created by kncm_ken@hotmail.co.th (kittipon.n)

// Define pins for buttons and LEDs
const int buttonPins[] = {2, 3, 4, 5};  // Pins for player buttons
const int ledPins[] = {6, 7, 8, 9};     // Pins for player LEDs
const int resetButtonPin1 = 10;         // Pin for first reset button
const int resetButtonPin2 = 12;         // Pin for second reset button
const int resetLEDPin = 11;             // Pin for reset LED
const int standbyLEDPin = 13;           // Pin for standby LED

// Define variables
int winner = -1;               // Player who has won (-1 means no winner)
bool gameEnded = false;        // Flag to indicate if the game has ended
unsigned long previousMillis = 0;  // Store the last time the LED was updated
const long interval = 300;     // Interval for blinking (300 milliseconds)
unsigned long debounceDelay = 50;  // Button debounce time in milliseconds
unsigned long buttonPressTimes[4]; // Store the last time a button was pressed

void setup() {
  // Initialize player buttons as inputs with pull-up resistors
  for (int i = 0; i < 4; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
    buttonPressTimes[i] = 0; // Initialize button press times
  }
  
  // Initialize player LEDs as outputs
  for (int i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW); // Ensure all LEDs are off initially
  }
  
  // Initialize reset buttons and LED pins
  pinMode(resetButtonPin1, INPUT_PULLUP);
  pinMode(resetButtonPin2, INPUT_PULLUP);
  pinMode(resetLEDPin, OUTPUT);
  pinMode(standbyLEDPin, OUTPUT);
  
  // Turn on the standby LED initially
  digitalWrite(standbyLEDPin, HIGH);
}

void loop() {
  // Check if game has ended
  if (!gameEnded) {
    checkPlayerButtons(); // Check for player button presses
  }

  // If a winner is declared, handle the end of the game
  if (gameEnded) {
    displayWinner();
    blinkResetLED(); // Blink reset LED
    digitalWrite(standbyLEDPin, LOW); // Turn off standby LED
  } else {
    digitalWrite(resetLEDPin, LOW); // Ensure reset LED is off if game hasn't ended
  }

  // Check for reset button press (either of the two reset buttons) to start a new game
  if (digitalRead(resetButtonPin1) == LOW || digitalRead(resetButtonPin2) == LOW) {
    resetGame();
  }
}

void checkPlayerButtons() {
  for (int i = 0; i < 4; i++) {
    if (digitalRead(buttonPins[i]) == LOW) {
      unsigned long currentMillis = millis();

      // Check for button debouncing
      if (currentMillis - buttonPressTimes[i] >= debounceDelay) {
        winner = i; // Player i has won
        gameEnded = true;
        break; // Exit loop once a winner is found
      }
      
      buttonPressTimes[i] = currentMillis; // Update the last press time
    }
  }
}

void displayWinner() {
  // Turn on winner's LED and disable others
  for (int i = 0; i < 4; i++) {
    if (i == winner) {
      digitalWrite(ledPins[i], HIGH); // Light up the winner's LED
    } else {
      digitalWrite(ledPins[i], LOW);  // Turn off other LEDs
      pinMode(buttonPins[i], INPUT);  // Disable other player buttons
    }
  }
}

void blinkResetLED() {
  unsigned long currentMillis = millis();
  
  // Toggle the reset LED at regular intervals
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    digitalWrite(resetLEDPin, !digitalRead(resetLEDPin)); // Toggle LED state
  }
}

void resetGame() {
  // Reset game state and re-enable player buttons
  for (int i = 0; i < 4; i++) {
    digitalWrite(ledPins[i], LOW);  // Turn off player LEDs
    pinMode(buttonPins[i], INPUT_PULLUP); // Re-enable buttons
  }

  // Reset the winner and game status
  winner = -1;
  gameEnded = false;

  // Reset the reset and standby LEDs
  digitalWrite(resetLEDPin, LOW);   // Turn off reset LED
  digitalWrite(standbyLEDPin, HIGH); // Turn on standby LED
}
