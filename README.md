# QuickLight Arena

QuickLight Arena is a multi-player reaction game built for Arduino. Players compete to be the first to press their button when the game starts, with LED indicators showing the winner.

## Features

- Supports up to 4 players
- LED indicators for each player and game status
- Continuous reset functionality for quick game restarts
- Debounced inputs for reliable button presses

## Hardware Requirements

- 1x Arduino board (e.g., Arduino Uno)
- 4x Push buttons (one for each player)
- 2x Push buttons for reset
- 6x LEDs (4 for players, 1 for reset, 1 for standby)
- 6x 220Ω resistors (for LEDs)
- Jumper wires
- Breadboard (optional, for prototyping)

## Pin Configuration

- Player Buttons: Digital pins 2, 3, 4, 5
- Player LEDs: Digital pins 6, 7, 8, 9
- Reset Button 1: Digital pin 10
- Reset Button 2: Digital pin 12
- Reset LED: Digital pin 11
- Standby LED: Digital pin 13

## Installation

1. Clone this repository or download the source code.
2. Open the `button_blitz_version__2_2.ino` file in the Arduino IDE. (also the latest version)
3. Connect your Arduino board to your computer.
4. Select the correct board and port in the Arduino IDE.
5. Click the "Upload" button to flash the code to your Arduino.

## How to Play

1. When the game starts, the Standby LED will be lit.
2. Players should place their fingers on their respective buttons.
3. When the Standby LED turns off, the first player to press their button wins.
4. The winner's LED will light up.
5. To start a new game, press and hold either reset button for at least 300ms.
6. Holding the reset button will cause the game to reset repeatedly.

## Customization

We need a bigger Arduino board like Mega Family.

You can adjust the following constants in the code to customize the game behavior:

- `NUM_PLAYERS`: Change the number of supported players (default: 4)
- `RESET_HOLD_TIME`: Adjust the time required to hold the reset button for the reset (default: 300ms)

## Contributing

Contributions to QuickLight Arena are welcome! Please feel free to submit a Pull Request.

## License

This project is open source and available under the [MIT License](LICENSE).
