// =======================================================
// Game Controller v3.3.0 (Improved & Hardened)
// Author: Kittipon N (kncm_ken@hotmail.co.th)
// QuickLight-Arena-4-players
// =======================================================

// -------------------- Pin Definitions ------------------
const int buttonPins[]      = {2, 3, 4, 5};
const int ledPins[]         = {6, 7, 8, 9};
const int resetButtonPins[] = {10, 12};

const int resetLEDPin   = 11;
const int standbyLEDPin = 13;
const int footSwitchPin = A5;
const int sirenRelayPin = A4;

// -------------------- Constants ------------------------
const int NUM_PLAYERS       = 4;
const int NUM_RESET_BUTTONS = 2;
const unsigned long debounceDelay = 50;

// Debounce index mapping
#define RESET_BTN1_IDX   0
#define RESET_BTN2_IDX   1
#define FOOT_SWITCH_IDX  2
#define SPARE_IDX        3

// -------------------- Variables ------------------------
int winner = -1;
bool gameEnded = false;
unsigned long lastDebounceTimes[4] = {0, 0, 0, 0};

// ======================================================
// SETUP
// ======================================================
void setup() {
  Serial.begin(9600);
  Serial.println("Game Controller v3.2.0 Starting...");

  for (int i = 0; i < NUM_PLAYERS; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
  }

  for (int i = 0; i < NUM_RESET_BUTTONS; i++) {
    pinMode(resetButtonPins[i], INPUT_PULLUP);
  }

  pinMode(resetLEDPin, OUTPUT);
  pinMode(standbyLEDPin, OUTPUT);
  pinMode(footSwitchPin, INPUT_PULLUP);
  pinMode(sirenRelayPin, OUTPUT);

  setIdleState();
  Serial.println("Game ready - waiting for players...");
}

// ======================================================
// MAIN LOOP
// ======================================================
void loop() {
  if (!gameEnded) {
    if (digitalRead(footSwitchPin) == HIGH) {
      checkPlayerButtons();
    }
  } else {
    maintainGameEndState();
  }

  checkResetInputs();
}

// ======================================================
// PLAYER BUTTON HANDLING
// ======================================================
void checkPlayerButtons() {
  for (int i = 0; i < NUM_PLAYERS; i++) {
    if (digitalRead(buttonPins[i]) == LOW && winner == -1) {
      declareWinner(i);
      break;
    }
  }
}

void declareWinner(int playerIndex) {
  winner = playerIndex;
  gameEnded = true;

  Serial.print("Player ");
  Serial.print(playerIndex + 1);
  Serial.println(" wins!");

  digitalWrite(standbyLEDPin, LOW);
  digitalWrite(resetLEDPin, HIGH);
  digitalWrite(sirenRelayPin, HIGH);

  updateWinnerLEDs();
}

// ======================================================
// GAME END STATE
// ======================================================
void maintainGameEndState() {
  updateWinnerLEDs();
}

void updateWinnerLEDs() {
  for (int i = 0; i < NUM_PLAYERS; i++) {
    digitalWrite(ledPins[i], (i == winner) ? HIGH : LOW);
  }
}

// ======================================================
// RESET HANDLING
// ======================================================
void checkResetInputs() {
  unsigned long now = millis();

  if (digitalRead(resetButtonPins[0]) == LOW &&
      now - lastDebounceTimes[RESET_BTN1_IDX] > debounceDelay) {
    lastDebounceTimes[RESET_BTN1_IDX] = now;
    Serial.println("Reset button 1 pressed");
    resetGame();
  }

  if (digitalRead(resetButtonPins[1]) == LOW &&
      now - lastDebounceTimes[RESET_BTN2_IDX] > debounceDelay) {
    lastDebounceTimes[RESET_BTN2_IDX] = now;
    Serial.println("Reset button 2 pressed");
    resetGame();
  }

  if (digitalRead(footSwitchPin) == LOW &&
      now - lastDebounceTimes[FOOT_SWITCH_IDX] > debounceDelay) {
    lastDebounceTimes[FOOT_SWITCH_IDX] = now;
    Serial.println("Foot switch reset activated");
    resetGame();
  }
}

// ======================================================
// RESET GAME
// ======================================================
void resetGame() {
  Serial.println("Resetting game...");

  winner = -1;
  gameEnded = false;

  for (int i = 0; i < NUM_PLAYERS; i++) {
    digitalWrite(ledPins[i], LOW);
  }

  setIdleState();
  Serial.println("Game reset complete - ready for new game");
}

// ======================================================
// LED / OUTPUT STATES
// ======================================================
void setIdleState() {
  digitalWrite(resetLEDPin, LOW);
  digitalWrite(standbyLEDPin, HIGH);
  digitalWrite(sirenRelayPin, LOW);
}
