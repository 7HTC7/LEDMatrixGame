# Arduino LED Matrix Games

Three classic games — **Tetris**, **Snake** and **Plane** (a small shooter) — running on an 8x32 LED matrix (four MAX7219 modules), 
with the score shown on a 3-digit 7-segment display.

## Features

- Three games in one sketch, switchable with a button
- 8x32 LED matrix display (4x MAX7219 8x8 modules chained together)
- 3-digit 7-segment score display (max 999)
- Joystick control, action button and passive buzzer for sound effects
- Pause function and flashing "game over" screen

## Hardware

- Arduino Uno
- 4x MAX7219 8x8 LED matrix modules (chained)
- 3-digit 7-segment display (common cathode)
- Analog joystick
- 2x push buttons (action button and mode button)
- Passive buzzer
- Resistors for the 7-segment display, jumper wires, breadboard

## Controls

| Input | What it does |
| Mode button (short press) | Pause / resume |
| Mode button (hold 2 seconds) | Switch game: Tetris → Snake → Plane |
| Action button | Game-specific (see below), and restarts the game after game over |
| Joystick | Game-specific (see below) |

### Tetris

- Joystick left / right: move the piece
- Joystick down: drop the piece faster
- Action button: rotate
- Score: 10 points per cleared line

### Snake

- Joystick: steer the snake
- Eat the food to grow and speed up
- Score: 1 point per food

### Plane

- Joystick up / down: move the plane left / right
- Joystick left / right: move the plane back / forward
- Action button: shoot
- Score: 1 point per enemy shot down
- The game ends if an enemy hits you

