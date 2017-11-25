/*
# Blink LEDs using delays

Simplest LED blinking example using the HAL library and software delays for timing
The code just sets up the pins as outputs and toggles the 4 LEDs in a pattern, using the builtin HAL_Delay() function for timing.


Developed for the STM32F4 Discovery board, but can be easily modified to run on any.
Just need to modify the LED output channels and/or attach some external LEDs.
 */
#include "stm32f4xx.h"
#include "stm32f4_discovery.h"

// Flag to toggle different methods of setting up GPIO pins. See GPIO_Init() function
#define COMBINED_SETUP

void GPIO_Init(void);

int main(void)
{
	/* Must be called first at the start of any code using the HAL library */
	HAL_Init();
	/* Setup the GPIO pins connected to the LEDs as outputs */
	GPIO_Init();

	/* Output a simple pattern to the LEDs.
	 * This simple example is implemented as delays, rather than with interrupts.
	 */
	while(1){
		HAL_GPIO_TogglePin(LED3_GPIO_PORT, LED3_PIN);
		HAL_Delay(100);
		HAL_GPIO_TogglePin(LED4_GPIO_PORT, LED4_PIN);
		HAL_Delay(100);
		HAL_GPIO_TogglePin(LED5_GPIO_PORT, LED5_PIN);
		HAL_Delay(100);
		HAL_GPIO_TogglePin(LED6_GPIO_PORT, LED6_PIN);
		HAL_Delay(100);
	}
}


/*
 * Setup the 4 LEDS on the Discovery board as outputs
 * The macros/function and pin numbers/ports (eg. LED4_PIN, LED4_GPIO_PORT)
 *   are defined in Utilities/STM32F4-Discovery/stm43f4_discovery.c
 *
 *   Basically, there are 2 steps:
 *    - Enable the GPIO clock for each Port
 *    	eg. __HAL_RCC_GPIOD_CLK_ENABLE();
 *    - Create a GPIO_InitTypeDef structure with the initialisation configuration for each pin
 *      - Pass this struct to HAL_GPIO_INIT function to set up each pin.
 *      eg. HAL_GPIO_Init(LED6_GPIO_PORT, &GPIO_InitStructure);
 *
 *    Note that if all the outputs are on the same port then they cen be set up at the same time.
 *    Both individual and grouped setup are shown, controlled by the COMBINED_SETUP #define
 */
void GPIO_Init(void){
	/* Create an InitStructure to hold the initialisation configuration for each pin */
	GPIO_InitTypeDef GPIO_InitStructure;

	// Common settings for all LEDs
	GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;	// Push-pull mode
	GPIO_InitStructure.Pull = GPIO_NOPULL;			// no pullup resistors
	GPIO_InitStructure.Speed = GPIO_SPEED_FREQ_LOW; // low frequency - not important for this example.

	/* Now we have to apply these settings to each pin
	 * In this case all 4 LEDS are on the same port so we can do this in one step
	 * eg.
	 */
#ifdef COMBINED_SETUP
	__HAL_RCC_GPIOD_CLK_ENABLE();   // All 4 LEDS are on PORTD so we can just enable the clock once for that port
	GPIO_InitStructure.Pin = LED3_PIN | LED4_PIN | LED5_PIN | LED6_PIN; //We can set up all pins at once by or-ing them together
    HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);
#else
    /* The more general approach is to set up each pin individually */
    LED3_GPIO_CLK_ENABLE();
    LED4_GPIO_CLK_ENABLE();
    LED5_GPIO_CLK_ENABLE();
    LED6_GPIO_CLK_ENABLE();
	GPIO_InitStructure.Pin = LED3_PIN;
    HAL_GPIO_Init(LED3_GPIO_PORT, &GPIO_InitStructure);
	GPIO_InitStructure.Pin = LED4_PIN;					//reuse the same structure. Just update the pin number
    HAL_GPIO_Init(LED4_GPIO_PORT, &GPIO_InitStructure);
	GPIO_InitStructure.Pin = LED5_PIN;					//reuse the same structure. Just update the pin number
    HAL_GPIO_Init(LED5_GPIO_PORT, &GPIO_InitStructure);
	GPIO_InitStructure.Pin = LED6_PIN;					//reuse the same structure. Just update the pin number
    HAL_GPIO_Init(LED6_GPIO_PORT, &GPIO_InitStructure);
#endif
}
