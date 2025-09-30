// Created by kncm_ken@hotmail.co.th (Kittipon N)
// Modified on September 2025 - Stability fix version


// Pin definitions
const int buttonPins[] = {2, 3, 4, 5};  // Player button pins
const int ledPins[] = {6, 7, 8, 9};     // Player LED pins
const int resetButtonPins[] = {10, 12}; // Reset buttons
const int resetLEDPin = 11;             // Reset LED
const int standbyLEDPin = 13;           // Standby LED
const int footSwitchPin = A5;           // Foot switch
const int sirenRelayPin = A4;           // Siren relay trigger output on A4

// Constants - ADJUSTED FOR STABILITY
const long resetDebounceDelay = 200;    // Longer debounce for reset buttons (in ms)
const int stableReadings = 10;          // Increased for better stability
const long buttonHoldTime = 100;        // Increased minimum hold time (in ms)
const int loopDelay = 5;                // Small delay in main loop for stability

// Variables
bool playerLEDStates[] = {false, false, false, false};      // Track each player's LED state
int consecutiveLowCount[] = {0, 0, 0, 0};                  // Count consecutive LOW readings
unsigned long firstLowTime[4] = {0, 0, 0, 0};              // When button first went LOW
bool anyPlayerActive = false;                               // Track if any player is active
unsigned long resetButtonDebounce[2] = {0, 0};             // Debounce timers for reset buttons  
unsigned long footSwitchDebounce = 0;                      // Debounce timer for foot switch
bool resetInProgress = false;                               // Prevent multiple resets

// Siren control (disabled)
bool sirenEnabled = false;              // Set to true if you want siren functionality

void setup() {
  // Initialize button and LED pins
  for (int i = 0; i < 4; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);  // Ensure all LEDs start OFF
  }

  for (int i = 0; i < 2; i++) {
    pinMode(resetButtonPins[i], INPUT_PULLUP);
  }

  pinMode(resetLEDPin, OUTPUT);
  pinMode(standbyLEDPin, OUTPUT);
  pinMode(footSwitchPin, INPUT_PULLUP);
  pinMode(sirenRelayPin, OUTPUT);
  
  digitalWrite(standbyLEDPin, HIGH);    // Turn on standby LED initially
  digitalWrite(resetLEDPin, LOW);       // Reset LED starts OFF
  digitalWrite(sirenRelayPin, LOW);     // Initialize siren relay as OFF
  
  anyPlayerActive = false;
  resetInProgress = false;
  
  // Small startup delay for hardware stabilization
  delay(100);
}

void loop() {
  // Check reset inputs first to ensure they have priority
  checkResetButtons();
  
  // Only process player buttons if not currently resetting
  if (!resetInProgress) {
    // Check player buttons only if foot switch is not pressed
    if (digitalRead(footSwitchPin) == HIGH) {
      checkPlayerButtons();
    }
  }
  
  // Update system state based on active players
  updateSystemState();
  
  // CRITICAL: Small delay for stable readings
  delay(loopDelay);
}

// Function to check player buttons - IMPROVED ANTI-FLASH VERSION
void checkPlayerButtons() {
  unsigned long currentMillis = millis();
  
  // Loop through ALL 4 players
  for (int i = 0; i < 4; i++) {
    // Skip if LED is already ON for this player
    if (playerLEDStates[i]) {
      continue;
    }
    
    int buttonState = digitalRead(buttonPins[i]);
    
    // Button is pressed (LOW)
    if (buttonState == LOW) {
      // If this is the first LOW reading, record the time
      if (consecutiveLowCount[i] == 0) {
        firstLowTime[i] = currentMillis;
      }
      
      // Increment consecutive LOW count
      consecutiveLowCount[i]++;
      
      // Check if ALL conditions are met:
      // 1. Enough consecutive LOW readings
      // 2. Button held long enough
      // 3. Time check to ensure readings happened over actual time (not instant)
      if (consecutiveLowCount[i] >= stableReadings && 
          (currentMillis - firstLowTime[i]) >= buttonHoldTime) {
        
        // ALL CONDITIONS MET - Turn on LED permanently
        playerLEDStates[i] = true;
        digitalWrite(ledPins[i], HIGH);
        
        // Reset counter
        consecutiveLowCount[i] = 0;
      }
    } 
    // Button is released (HIGH) or not pressed
    else {
      // Button released before conditions met - reset counter, NO LED CHANGE
      if (consecutiveLowCount[i] > 0) {
        consecutiveLowCount[i] = 0;
        firstLowTime[i] = 0;
      }
    }
  }
}

// Update system state based on active players
void updateSystemState() {
  // Check if any player is active
  anyPlayerActive = false;
  for (int i = 0; i < 4; i++) {
    if (playerLEDStates[i]) {
      anyPlayerActive = true;
      break;
    }
  }
  
  // Update standby LED and reset LED
  if (anyPlayerActive) {
    digitalWrite(standbyLEDPin, LOW);     // Turn off standby LED when players are active
    digitalWrite(resetLEDPin, HIGH);      // Turn on reset LED to indicate reset is available
    if (sirenEnabled) {
      digitalWrite(sirenRelayPin, HIGH);  // Activate siren when any player is active (only if enabled)
    }
  } else {
    digitalWrite(standbyLEDPin, HIGH);    // Turn on standby LED when no players are active
    digitalWrite(resetLEDPin, LOW);       // Turn off reset LED when no players are active
    digitalWrite(sirenRelayPin, LOW);     // Ensure siren is off when no players are active
  }
}

// Check if any reset button or foot switch is pressed - IMPROVED
void checkResetButtons() {
  unsigned long currentMillis = millis();

  // Prevent checking during reset
  if (resetInProgress) {
    return;
  }

  // Check reset buttons with debounce
  for (int i = 0; i < 2; i++) {
    if (digitalRead(resetButtonPins[i]) == LOW) {
      if ((currentMillis - resetButtonDebounce[i]) > resetDebounceDelay) {
        resetButtonDebounce[i] = currentMillis;
        resetGame();
        return; // Exit after reset
      }
    }
  }

  // Foot switch as reset - ONLY when pressed deliberately
  // Must be held LOW continuously
  if (digitalRead(footSwitchPin) == LOW) {
    if ((currentMillis - footSwitchDebounce) > resetDebounceDelay) {
      footSwitchDebounce = currentMillis;
      resetGame();
    }
  } else {
    // Reset the foot switch debounce timer when not pressed
    footSwitchDebounce = currentMillis;
  }
}

// Reset the game state and turn off all LEDs
void resetGame() {
  resetInProgress = true;
  
  // Reset ALL 4 players
  for (int i = 0; i < 4; i++) {
    digitalWrite(ledPins[i], LOW);
    playerLEDStates[i] = false;
    consecutiveLowCount[i] = 0;
    firstLowTime[i] = 0;
  }

  // Reset system state
  anyPlayerActive = false;
  digitalWrite(resetLEDPin, LOW);
  digitalWrite(standbyLEDPin, HIGH);
  digitalWrite(sirenRelayPin, LOW);  // Turn off siren relay on reset
  
  // Small delay to ensure reset completes
  delay(50);
  
  resetInProgress = false;
}
