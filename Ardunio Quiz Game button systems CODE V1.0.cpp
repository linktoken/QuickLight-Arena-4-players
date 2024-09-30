//Code version 1.0
//Create by kncm_ken@hotmail.co.th (kittipon.n)

// Define pins for buttons and LEDs
const int buttonPins[] = {2, 3, 4, 5};  // Pins for player buttons
const int ledPins[] = {6, 7, 8, 9};     // Pins for player LEDs
const int resetButtonPin = 10;          // Pin for reset button
const int resetLEDPin = 11;             // Pin for reset LED
const int standbyLEDPin = 13;           // Pin for standby LED

// Define variables
int winner = -1; // Player who has won (-1 means no winner)
bool gameEnded = false; // Flag to indicate if the game has ended
unsigned long previousMillis = 0; // Store the last time the LED was updated
const long interval = 300; // Interval for blinking (300 milliseconds)

void setup() {
  // Initialize button pins as inputs with pull-up resistors
  for (int i = 0; i < 4; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
  
  // Initialize LED pins as outputs
  for (int i = 0; i < 4; i++) {
    pinMode(ledPins[i], OUTPUT);
  }
  
  // Initialize reset button pin as input with pull-up resistor
  pinMode(resetButtonPin, INPUT_PULLUP);
  
  // Initialize reset LED pin as output
  pinMode(resetLEDPin, OUTPUT);

  // Initialize standby LED pin as output
  pinMode(standbyLEDPin, OUTPUT);
  digitalWrite(standbyLEDPin, HIGH); // Turn on standby LED initially
}

void loop() {
  // Check if game has ended
  if (!gameEnded) {
    // Check each player's button
    for (int i = 0; i < 4; i++) {
      if (digitalRead(buttonPins[i]) == LOW) {
        // Player i has pressed the button
        winner = i;
        gameEnded = true;
        break;
      }
    }
  }
  
  // If a winner is declared, light up their LED and disable other buttons
  if (gameEnded) {
    for (int i = 0; i < 4; i++) {
      if (i == winner) {
        digitalWrite(ledPins[i], HIGH); // Light up winner's LED
      } else {
        digitalWrite(ledPins[i], LOW);  // Turn off other LEDs
        pinMode(buttonPins[i], INPUT);  // Disable other buttons
      }
    }
    
    // Blink the reset LED
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      // Toggle the reset LED
      if (digitalRead(resetLEDPin) == LOW) {
        digitalWrite(resetLEDPin, HIGH);
      } else {
        digitalWrite(resetLEDPin, LOW);
      }
    }
    
    digitalWrite(standbyLEDPin, LOW); // Turn off standby LED
  } else {
    digitalWrite(resetLEDPin, LOW); // Ensure reset LED is off if game hasn't ended
  }
  
  // Check for reset button press to start a new game
  if (digitalRead(resetButtonPin) == LOW) {
    resetGame();
  }
}

void resetGame() {
  // Turn off all LEDs
  for (int i = 0; i < 4; i++) {
    digitalWrite(ledPins[i], LOW);
    pinMode(buttonPins[i], INPUT_PULLUP); // Re-enable buttons
  }
  
  // Reset game state
  winner = -1;
  gameEnded = false;
  digitalWrite(resetLEDPin, LOW); // Turn off reset LED
  digitalWrite(standbyLEDPin, HIGH); // Turn on standby LED
}
