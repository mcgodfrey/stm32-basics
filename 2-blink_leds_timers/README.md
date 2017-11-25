# Blink LEDs using timer interrupts

Basic example of blinking LEDs with interrupts.
 - Uses a single timer (TIM3)
 - 3 LEDs connected to 3 Output Compare (OC) channels (4th channel unused)
 - 1 LED connected to timer overflow interrupt

The timer is adjusted to overflow roughly every second (using default clock setup with 16MHz internal RC oscillator).
The 3 OC channels are set to trigger at different points in the timer ramp, and the interrupt function simply toggles the LEDs, as does the main timer overflow interrupt.

Developed for the STM32F4 Discovery board, but can be easily modified to run on any.
Just need to modify the LED output channels and/or attach some external LEDs.