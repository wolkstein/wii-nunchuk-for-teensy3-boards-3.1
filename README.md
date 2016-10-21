# wii nunchuk on teensy3.1 or teensy3.2 boards

An arduino example to use the wii nunchuk controller with an teensy3.1/3.2 board without external pull-up resistors and no timing problems.

With this example you can use any cpu speed on tennsy3.1 or 3.2

This repository contains an wii nunchuk example ino and an modified version of Nunchuk.h and Nunchuk.cpp

The example ino contains everything at once(mouse, keyboard and joystick use cases).

First, It can use the Nunchuk as Joystick. Simple press the big Nunchuk BTN around two seconds during initialisation (while plugging the teensy into usb jack).

Or second the default setup as mouse keyboard combination with left and right beat gestures from accel X sensor mapped to Key_Left/Key_Right. Beside Joystick XY for mouse XY movements and BTN 1 2 for mouse left and right clicking.

In this example the Nunchuk class is included locally. No need to copy the Nunchuck Class to the arduino library's folder.

# How this example work:

A timer function check and read all of the Nunchuk controller data all 10ms and write them into global volatile variables with the data type used by Nunchuk.h.

In init function we init Nunchk with nc.begin() and start the timer function to read all nunchuk data each 10 ms. Than we calibrate two seconds the Joystick offset X Y for the Poti based Nunchuk Joystik. I noticed that the joystick is not very accurate around the middle position. Than we create an lookup table for this joystick to define an dead-zone. Default set to 5 (int mouseDead = 5;). I called this mouseDead because it is only used by mouse emulation. I noticed that most joystick calibration software from your OS will allow you to define your own dead-zone.

In main Loop we process the the Nunchuk data to the different use cases. First is Keyboard, second is Joystick and third is Mouse Emulation.

at last we have some helper functions to get an accelerate X Y average, calculate the left and right beat gestures from accel X sensor.
