// QuickLight Arena
// A multi-player reaction game with LED indicators
// Version 2.3
// Original concept by kncm_ken@hotmail.co.th (kittipon n.)
// Improvements and naming by Claude

#include <Arduino.h>

// Constants
const uint8_t NUM_PLAYERS = 4;
const uint8_t BUTTON_PINS[NUM_PLAYERS] = {2, 3, 4, 5};
const uint8_t LED_PINS[NUM_PLAYERS] = {6, 7, 8, 9};
const uint8_t RESET_BUTTON_PIN_1 = 10;
const uint8_t RESET_BUTTON_PIN_2 = 12;
const uint8_t RESET_LED_PIN = 11;
const uint8_t STANDBY_LED_PIN = 13;
const unsigned long BLINK_INTERVAL = 300; // milliseconds
const unsigned long DEBOUNCE_DELAY = 50; // milliseconds
const unsigned long RESET_HOLD_TIME = 300; // milliseconds to hold for initial reset
const unsigned long RESET_REPEAT_INTERVAL = 500; // milliseconds between repeated resets

// Game state
enum GameState {
  STANDBY,
  PLAYING,
  ENDED
};

GameState currentState = STANDBY;
int8_t winner = -1;
unsigned long lastBlinkTime = 0;
unsigned long lastDebounceTime = 0;
unsigned long resetPressStartTime = 0;
unsigned long lastResetTime = 0;
bool lastResetButtonState = HIGH;
bool resetButtonState = HIGH;

void setup() {
  for (uint8_t i = 0; i < NUM_PLAYERS; i++) {
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);
    pinMode(LED_PINS[i], OUTPUT);
  }
  
  pinMode(RESET_BUTTON_PIN_1, INPUT_PULLUP);
  pinMode(RESET_BUTTON_PIN_2, INPUT_PULLUP);
  pinMode(RESET_LED_PIN, OUTPUT);
  pinMode(STANDBY_LED_PIN, OUTPUT);
  
  resetGame();
}

void loop() {
  switch (currentState) {
    case STANDBY:
      handleStandbyState();
      break;
    case PLAYING:
      handlePlayingState();
      break;
    case ENDED:
      handleEndedState();
      break;
  }
  
  handleResetButton();
}

void handleStandbyState() {
  digitalWrite(STANDBY_LED_PIN, HIGH);
  if (isAnyPlayerButtonPressed()) {
    currentState = PLAYING;
    digitalWrite(STANDBY_LED_PIN, LOW);
  }
}

void handlePlayingState() {
  for (uint8_t i = 0; i < NUM_PLAYERS; i++) {
    if (digitalRead(BUTTON_PINS[i]) == LOW) {
      winner = i;
      currentState = ENDED;
      break;
    }
  }
}

void handleEndedState() {
  for (uint8_t i = 0; i < NUM_PLAYERS; i++) {
    digitalWrite(LED_PINS[i], i == winner ? HIGH : LOW);
  }
  
  if (millis() - lastBlinkTime >= BLINK_INTERVAL) {
    lastBlinkTime = millis();
    digitalWrite(RESET_LED_PIN, !digitalRead(RESET_LED_PIN));
  }
}

bool isAnyPlayerButtonPressed() {
  for (uint8_t i = 0; i < NUM_PLAYERS; i++) {
    if (digitalRead(BUTTON_PINS[i]) == LOW) {
      return true;
    }
  }
  return false;
}

void handleResetButton() {
  bool reading = (digitalRead(RESET_BUTTON_PIN_1) == LOW || digitalRead(RESET_BUTTON_PIN_2) == LOW);

  if (reading != lastResetButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (reading != resetButtonState) {
      resetButtonState = reading;
      if (resetButtonState == LOW) {
        // Button has just been pressed
        resetPressStartTime = millis();
      } else {
        // Button has just been released
        resetPressStartTime = 0;
        lastResetTime = 0;
      }
    }
    
    // Check for initial reset
    if (resetButtonState == LOW && (millis() - resetPressStartTime) >= RESET_HOLD_TIME && lastResetTime == 0) {
      resetGame();
      lastResetTime = millis();
    }
    
    // Check for repeated resets
    if (resetButtonState == LOW && lastResetTime != 0 && (millis() - lastResetTime) >= RESET_REPEAT_INTERVAL) {
      resetGame();
      lastResetTime = millis();
    }
  }

  lastResetButtonState = reading;
}

void resetGame() {
  for (uint8_t i = 0; i < NUM_PLAYERS; i++) {
    digitalWrite(LED_PINS[i], LOW);
  }
  
  winner = -1;
  currentState = STANDBY;
  digitalWrite(RESET_LED_PIN, LOW);
  digitalWrite(STANDBY_LED_PIN, HIGH);
}