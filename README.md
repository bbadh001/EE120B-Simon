# EE120B Final Project: “Simon…on a breadboard!”
Brent Badhwar

Summer 2017

## Overview:

“Simon…on a breadboard!” is a throwback to the hand-held game “Simon” released in
the late 70’s. “Simon” is a memory game, requiring you to duplicate the random series of light
patterns displayed by the computer. On each successive round, a new light is added to the
series for you to copy. Once you fail to copy the series correctly, it’s game over!

## Hardware Components:

-4 LED’s (used to display the lighting sequence)
-5 buttons (4 to trigger each their respective LED and one as a start/continue button)
-LCD display (to display the player’s score and whether they won or lost)
-AtMega1284 microcontroller

## How to Play:

Once the breadboard is powered on, you will be met with the welcome screen on the
LCD Display. When you are ready to play, press the start button. The game will now begin by
showing the first the LED for the random sequence. Each round, you will try to replicate the
LED sequence shown by pressing the corresponding buttons. If you survive to round 9, you
win the game. After the game ends, you may press the start button to start a new game.
