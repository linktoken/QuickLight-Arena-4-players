// QuickLight Arena
// A multi-player reaction game with LED indicators
// Version 3.0
// Original concept by kncm_ken@hotmail.co.th (kittipon n.)


#include <Arduino.h>

// Constants
const uint8_t NUM_PLAYERS = 4;
const uint8_t FOOT_SWITCH_PIN = 1;  // New foot switch pin
const uint8_t BUTTON_PINS[NUM_PLAYERS] = {2, 3, 4, 5};
const uint8_t LED_PINS[NUM_PLAYERS] = {6, 7, 8, 9};
const uint8_t RESET_BUTTON_PIN_1 = 10;
const uint8_t RESET_BUTTON_PIN_2 = 12;
const uint8_t RESET_LED_PIN = 11;
const uint8_t STANDBY_LED_PIN = 13;

// Timing constants
const unsigned long BLINK_INTERVAL = 300;     // milliseconds
const unsigned long DEBOUNCE_DELAY = 50;      // milliseconds
const unsigned long RESET_HOLD_TIME = 100;    // milliseconds to hold for reset
const unsigned long RESET_BLOCK_TIME = 1000;  // milliseconds to block reset after game start
const unsigned long WINNER_CELEBRATION_TIME = 3000; // milliseconds to show winner before auto-reset
const unsigned long START_DELAY_MIN = 1000;   // minimum milliseconds before game starts
const unsigned long START_DELAY_MAX = 3000;   // maximum milliseconds before game starts

// Game states
enum GameState {
  STANDBY,
  COUNTDOWN,
  PLAYING,
  ENDED
};

// Game variables
GameState currentState = STANDBY;
int8_t winner = -1;
unsigned long lastBlinkTime = 0;
unsigned long lastDebounceTime = 0;
unsigned long resetPressStartTime = 0;
unsigned long gameStartTime = 0;
unsigned long stateChangeTime = 0;
bool lastResetButtonState = HIGH;
bool resetButtonState = HIGH;
bool resetPerformed = false;
bool isBlinkOn = false;

void setup() {
  randomSeed(analogRead(0));  // Initialize random seed
  
  // Initialize foot switch pin
  pinMode(FOOT_SWITCH_PIN, INPUT_PULLUP);
  
  // Initialize all pins
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
  unsigned long currentTime = millis();
  
  // Check if foot switch is pressed (LOW)
  bool buttonsBlocked = (digitalRead(FOOT_SWITCH_PIN) == LOW);
  
  switch (currentState) {
    case STANDBY:
      handleStandbyState(currentTime, buttonsBlocked);
      break;
    case COUNTDOWN:
      handleCountdownState(currentTime, buttonsBlocked);
      break;
    case PLAYING:
      handlePlayingState(buttonsBlocked);
      break;
    case ENDED:
      handleEndedState(currentTime);
      break;
  }
  
  handleResetButton();
}

void handleStandbyState(unsigned long currentTime, bool buttonsBlocked) {
  // Blink standby LED slowly
  if (currentTime - lastBlinkTime >= BLINK_INTERVAL * 2) {
    lastBlinkTime = currentTime;
    isBlinkOn = !isBlinkOn;
    digitalWrite(STANDBY_LED_PIN, isBlinkOn);
  }
  
  // Only check player buttons if not blocked
  if (!buttonsBlocked && isAnyPlayerButtonPressed()) {
    currentState = COUNTDOWN;
    stateChangeTime = currentTime;
    allLEDsOff();
  }
}

void handleCountdownState(unsigned long currentTime, bool buttonsBlocked) {
  // Random delay before game starts
  unsigned long startDelay = random(START_DELAY_MIN, START_DELAY_MAX + 1);
  
  if (currentTime - stateChangeTime < startDelay) {
    // During countdown, blink all LEDs rapidly
    if (currentTime - lastBlinkTime >= BLINK_INTERVAL / 2) {
      lastBlinkTime = currentTime;
      isBlinkOn = !isBlinkOn;
      setAllPlayerLEDs(isBlinkOn);
    }
    
    // Check for false starts only if buttons aren't blocked
    if (!buttonsBlocked && isAnyPlayerButtonPressed()) {
      // Player pressed too early - penalize by showing their LED and returning to standby
      for (uint8_t i = 0; i < NUM_PLAYERS; i++) {
        if (digitalRead(BUTTON_PINS[i]) == LOW) {
          allLEDsOff();
          digitalWrite(LED_PINS[i], HIGH);
          delay(1000);  // Show penalty for 1 second
          resetGame();
          return;
        }
      }
    }
  } else {
    // Start the game
    currentState = PLAYING;
    gameStartTime = currentTime;
    allLEDsOff();
    digitalWrite(STANDBY_LED_PIN, HIGH);  // Signal game is active
  }
}

void handlePlayingState(bool buttonsBlocked) {
  // Only check for winners if buttons aren't blocked
  if (!buttonsBlocked) {
    for (uint8_t i = 0; i < NUM_PLAYERS; i++) {
      if (digitalRead(BUTTON_PINS[i]) == LOW) {
        winner = i;
        currentState = ENDED;
        stateChangeTime = millis();
        break;
      }
    }
  }
}

void handleEndedState(unsigned long currentTime) {
  // Victory animation
  if (currentTime - lastBlinkTime >= BLINK_INTERVAL) {
    lastBlinkTime = currentTime;
    isBlinkOn = !isBlinkOn;
    
    if (winner >= 0) {
      digitalWrite(LED_PINS[winner], isBlinkOn);
      digitalWrite(RESET_LED_PIN, !isBlinkOn);
    }
  }
  
  // Auto-reset after celebration time
  if (currentTime - stateChangeTime >= WINNER_CELEBRATION_TIME) {
    resetGame();
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
  unsigned long currentTime = millis();
  bool reading = (digitalRead(RESET_BUTTON_PIN_1) == LOW || digitalRead(RESET_BUTTON_PIN_2) == LOW);
  
  if (reading != lastResetButtonState) {
    lastDebounceTime = currentTime;
  }
  
  if ((currentTime - lastDebounceTime) > DEBOUNCE_DELAY) {
    if (reading != resetButtonState) {
      resetButtonState = reading;
      if (resetButtonState == LOW) {
        resetPressStartTime = currentTime;
      } else {
        resetPressStartTime = 0;
        resetPerformed = false;
      }
    }
    
    if (resetButtonState == LOW && !resetPerformed) {
      if ((currentTime - resetPressStartTime) >= RESET_HOLD_TIME) {
        if (currentState != PLAYING || (currentTime - gameStartTime) >= RESET_BLOCK_TIME) {
          resetGame();
          resetPerformed = true;
        }
      }
    }
  }
  
  lastResetButtonState = reading;
}

void resetGame() {
  allLEDsOff();
  winner = -1;
  currentState = STANDBY;
  digitalWrite(RESET_LED_PIN, LOW);
  digitalWrite(STANDBY_LED_PIN, HIGH);
  lastBlinkTime = millis();
  isBlinkOn = false;
}

void allLEDsOff() {
  for (uint8_t i = 0; i < NUM_PLAYERS; i++) {
    digitalWrite(LED_PINS[i], LOW);
  }
  digitalWrite(RESET_LED_PIN, LOW);
  digitalWrite(STANDBY_LED_PIN, LOW);
}

void setAllPlayerLEDs(bool state) {
  for (uint8_t i = 0; i < NUM_PLAYERS; i++) {
    digitalWrite(LED_PINS[i], state);
  }
}