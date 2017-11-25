# Blink LEDs using delays

Simplest LED blinking example using the HAL library and software delays for timing
The code just sets up the pins as outputs and toggles the 4 LEDs in a pattern, using the builtin HAL_Delay() function for timing.


Developed for the STM32F4 Discovery board, but can be easily modified to run on any.
Just need to modify the LED output channels and/or attach some external LEDs.