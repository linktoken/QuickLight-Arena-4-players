// QuickLight Arena
// A multi-player reaction game with LED indicators
// Version 3.0
// Original concept by kncm_ken@hotmail.co.th (kittipon n.)

#include <Arduino.h>

// Pin Configuration
struct PinConfig {
  const uint8_t footSwitch = 1;
  const uint8_t playerButtons[4] = {2, 3, 4, 5};
  const uint8_t playerLEDs[4] = {6, 7, 8, 9};
  const uint8_t resetButton1 = 10;
  const uint8_t resetButton2 = 12;
  const uint8_t resetLED = 11;
  const uint8_t standbyLED = 13;
};

// Timing Configuration
struct TimingConfig {
  const unsigned long debounceDelay = 50;      // ms
  const unsigned long blinkInterval = 300;      // ms
  const unsigned long resetHoldTime = 100;      // ms
  const unsigned long resetBlockTime = 1000;    // ms
  const unsigned long celebrationTime = 3000;   // ms
  const unsigned long startDelayMin = 1000;     // ms
  const unsigned long startDelayMax = 5000;     // ms
  const unsigned long falseStartPenalty = 2000; // ms
  const unsigned long idleTimeout = 30000;      // ms
  const unsigned long errorBlinkTime = 500;     // ms
};

// Game States
enum class GameState {
  IDLE,
  STANDBY,
  COUNTDOWN,
  PLAYING,
  ENDED,
  ERROR
};

// Button States for Debouncing
struct ButtonState {
  bool current = HIGH;
  bool last = HIGH;
  bool debounced = HIGH;
  unsigned long lastDebounceTime = 0;
};

class ReactionGame {
private:
  const PinConfig pins;
  const TimingConfig timing;
  const uint8_t NUM_PLAYERS = 4;
  
  // Game state variables
  GameState currentState = GameState::STANDBY;
  int8_t winner = -1;
  unsigned long stateStartTime = 0;
  unsigned long lastBlinkTime = 0;
  unsigned long lastActivityTime = 0;
  bool isBlinkOn = false;
  uint32_t gameCount = 0;
  uint16_t errorCode = 0;
  
  // Button states
  ButtonState footSwitch;
  ButtonState playerButtons[4];
  ButtonState resetButtons[2];
  
  // Private methods
  void updateButton(ButtonState& state, bool reading) {
    if (reading != state.last) {
      state.lastDebounceTime = millis();
    }
    
    if ((millis() - state.lastDebounceTime) > timing.debounceDelay) {
      if (reading != state.current) {
        state.current = reading;
        if (reading != state.debounced) {
          state.debounced = reading;
          lastActivityTime = millis(); // Update activity timestamp
        }
      }
    }
    state.last = reading;
  }
  
  bool checkFalseStart() {
    if (!isFootSwitchPressed()) {
      for (uint8_t i = 0; i < NUM_PLAYERS; i++) {
        if (!playerButtons[i].debounced) { // Button pressed
          penalizePlayer(i);
          return true;
        }
      }
    }
    return false;
  }
  
  void penalizePlayer(uint8_t player) {
    allLEDsOff();
    // Blink penalty pattern
    for (uint8_t i = 0; i < 3; i++) {
      digitalWrite(pins.playerLEDs[player], HIGH);
      delay(timing.errorBlinkTime);
      digitalWrite(pins.playerLEDs[player], LOW);
      delay(timing.errorBlinkTime);
    }
    delay(timing.falseStartPenalty);
    resetGame();
  }
  
  void updateAllButtons() {
    // Update foot switch
    updateButton(footSwitch, digitalRead(pins.footSwitch));
    
    // Update player buttons
    for (uint8_t i = 0; i < NUM_PLAYERS; i++) {
      updateButton(playerButtons[i], digitalRead(pins.playerButtons[i]));
    }
    
    // Update reset buttons
    updateButton(resetButtons[0], digitalRead(pins.resetButton1));
    updateButton(resetButtons[1], digitalRead(pins.resetButton2));
  }
  
  bool isFootSwitchPressed() const {
    return !footSwitch.debounced;
  }
  
  bool isResetPressed() const {
    return !resetButtons[0].debounced || !resetButtons[1].debounced;
  }
  
  void handleIdleState() {
    // Dim all LEDs in idle state
    if (millis() - lastBlinkTime >= timing.blinkInterval * 4) {
      lastBlinkTime = millis();
      isBlinkOn = !isBlinkOn;
      digitalWrite(pins.standbyLED, isBlinkOn);
    }
    
    if (isAnyButtonPressed()) {
      wakeUp();
    }
  }
  
  void handleStandbyState() {
    // Blink standby LED
    if (millis() - lastBlinkTime >= timing.blinkInterval * 2) {
      lastBlinkTime = millis();
      isBlinkOn = !isBlinkOn;
      digitalWrite(pins.standbyLED, isBlinkOn);
    }
    
    // Check for game start
    if (!isFootSwitchPressed() && isAnyPlayerButtonPressed()) {
      startNewGame();
    }
  }
  
  void handleCountdownState() {
    unsigned long elapsed = millis() - stateStartTime;
    unsigned long startDelay = random(timing.startDelayMin, timing.startDelayMax);
    
    if (elapsed < startDelay) {
      // Rapid blink during countdown
      if (millis() - lastBlinkTime >= timing.blinkInterval / 2) {
        lastBlinkTime = millis();
        isBlinkOn = !isBlinkOn;
        setAllPlayerLEDs(isBlinkOn);
      }
      
      if (checkFalseStart()) {
        return;
      }
    } else {
      currentState = GameState::PLAYING;
      stateStartTime = millis();
      allLEDsOff();
      digitalWrite(pins.standbyLED, HIGH);
    }
  }
  
  void handlePlayingState() {
    if (!isFootSwitchPressed()) {
      for (uint8_t i = 0; i < NUM_PLAYERS; i++) {
        if (!playerButtons[i].debounced) {
          winner = i;
          currentState = GameState::ENDED;
          stateStartTime = millis();
          gameCount++;
          break;
        }
      }
    }
  }
  
  void handleEndedState() {
    if (millis() - lastBlinkTime >= timing.blinkInterval) {
      lastBlinkTime = millis();
      isBlinkOn = !isBlinkOn;
      
      if (winner >= 0) {
        digitalWrite(pins.playerLEDs[winner], isBlinkOn);
        digitalWrite(pins.resetLED, !isBlinkOn);
      }
    }
    
    if (millis() - stateStartTime >= timing.celebrationTime) {
      resetGame();
    }
  }
  
  void handleErrorState() {
    // Blink error code
    if (millis() - lastBlinkTime >= timing.errorBlinkTime) {
      lastBlinkTime = millis();
      isBlinkOn = !isBlinkOn;
      setAllLEDs(isBlinkOn);
    }
    
    if (isResetPressed()) {
      errorCode = 0;
      resetGame();
    }
  }
  
  void wakeUp() {
    currentState = GameState::STANDBY;
    lastActivityTime = millis();
    resetGame();
  }
  
  bool isAnyButtonPressed() const {
    if (!footSwitch.debounced) return true;
    for (uint8_t i = 0; i < NUM_PLAYERS; i++) {
      if (!playerButtons[i].debounced) return true;
    }
    return !resetButtons[0].debounced || !resetButtons[1].debounced;
  }
  
  bool isAnyPlayerButtonPressed() const {
    for (uint8_t i = 0; i < NUM_PLAYERS; i++) {
      if (!playerButtons[i].debounced) return true;
    }
    return false;
  }

public:
  ReactionGame() {
    // Constructor
    for (uint8_t i = 0; i < NUM_PLAYERS; i++) {
      pinMode(pins.playerButtons[i], INPUT_PULLUP);
      pinMode(pins.playerLEDs[i], OUTPUT);
    }
    
    pinMode(pins.footSwitch, INPUT_PULLUP);
    pinMode(pins.resetButton1, INPUT_PULLUP);
    pinMode(pins.resetButton2, INPUT_PULLUP);
    pinMode(pins.resetLED, OUTPUT);
    pinMode(pins.standbyLED, OUTPUT);
    
    randomSeed(analogRead(0));
  }
  
  void update() {
    updateAllButtons();
    
    // Check for idle timeout
    if (currentState != GameState::IDLE && 
        currentState != GameState::ERROR &&
        (millis() - lastActivityTime) > timing.idleTimeout) {
      currentState = GameState::IDLE;
    }
    
    // Handle reset button
    if (isResetPressed() && currentState != GameState::ERROR) {
      resetGame();
      return;
    }
    
    // State machine
    switch (currentState) {
      case GameState::IDLE:
        handleIdleState();
        break;
      case GameState::STANDBY:
        handleStandbyState();
        break;
      case GameState::COUNTDOWN:
        handleCountdownState();
        break;
      case GameState::PLAYING:
        handlePlayingState();
        break;
      case GameState::ENDED:
        handleEndedState();
        break;
      case GameState::ERROR:
        handleErrorState();
        break;
    }
  }
  
  void startNewGame() {
    currentState = GameState::COUNTDOWN;
    stateStartTime = millis();
    winner = -1;
    allLEDsOff();
  }
  
  void resetGame() {
    allLEDsOff();
    winner = -1;
    currentState = GameState::STANDBY;
    digitalWrite(pins.resetLED, LOW);
    digitalWrite(pins.standbyLED, HIGH);
    lastBlinkTime = millis();
    lastActivityTime = millis();
    isBlinkOn = false;
  }
  
  void setError(uint16_t code) {
    errorCode = code;
    currentState = GameState::ERROR;
    lastBlinkTime = millis();
  }
  
  void allLEDsOff() {
    setAllLEDs(false);
  }
  
  void setAllLEDs(bool state) {
    setAllPlayerLEDs(state);
    digitalWrite(pins.resetLED, state);
    digitalWrite(pins.standbyLED, state);
  }
  
  void setAllPlayerLEDs(bool state) {
    for (uint8_t i = 0; i < NUM_PLAYERS; i++) {
      digitalWrite(pins.playerLEDs[i], state);
    }
  }
  
  GameState getState() const {
    return currentState;
  }
  
  uint32_t getGameCount() const {
    return gameCount;
  }
  
  uint16_t getErrorCode() const {
    return errorCode;
  }
};

ReactionGame game;

void setup() {
  // Initialize game
}

void loop() {
  game.update();
}