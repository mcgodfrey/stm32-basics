/*
# Blink LEDs using timer interrupts

Basic example of blinking LEDs with interrupts.
 - Uses a single timer (TIM3)
 - 3 LEDs connected to 3 Output Compare (OC) channels (4th channel unused)
 - 1 LED connected to timer overflow interrupt

The timer is adjusted to overflow roughly every second (using default clock setup with 16MHz internal RC oscillator).
The 3 OC channels are set to trigger at different points in the timer ramp, and the interrupt function simply toggles the LEDs, as does the main timer overflow interrupt.

Developed for the STM32F4 Discovery board, but can be easily modified to run on any.
Just need to modify the LED output channels and/or attach some external LEDs.
 */
#include "stm32f4xx.h"
#include "stm32f4_discovery.h"

/* Global timer3 object.
 * This is accessed in initialisation, and in callback functions/interrupts
 */
TIM_HandleTypeDef htim3;

void GPIO_Init(void);
void TIM_Init(void);
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void TIM3_IRQHandler();


int main(void)
{
	/* Must be called first at the start of any code using the HAL library */
	HAL_Init();
	/* Setup the GPIO pins connected to the LEDs as outputs */
	GPIO_Init();
	/* Setup TIM3 timer to generate interrupts to toggle the LEDs */
	TIM_Init();


    /*
     * Start the timer (with interrupts)
     * I have 3 output compare (OC) channels (1, 2 and 3) which
     *   generate interrupts, and an interrupt on the main counter overflow
     */
    HAL_TIM_Base_Start_IT(&htim3);				// Main timer overflow
    HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_1);	// OC channel 1 interrupt
    HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_2);	// OC channel 2 interrupt
    HAL_TIM_OC_Start_IT(&htim3, TIM_CHANNEL_3);	// OC channel 3 interrupt

    /*
     * Main loop doesn't have to do anything now.
     * timerValue is just for debugging to see what the current timer count value is
     */
	while(1){
		int timerValue = __HAL_TIM_GET_COUNTER(&htim3);
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
	__HAL_RCC_GPIOD_CLK_ENABLE();   // All 4 LEDS are on PORTD so we can just enable the clock once for that port
	GPIO_InitStructure.Pin = LED3_PIN | LED4_PIN | LED5_PIN | LED6_PIN; //We can set up all pins at once by or-ing them together
    HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);

}



/*
 * Set up the TIM3 timer to generate interrupts to control the LEDs
 * I set the prescaler/period such that the overflow period is ~1s
 * Then set the output compare for each channel (1-3). I just hard code different
 *   values (10000, 35000 and 50000) as an example.
 * Finally, enable the interrupt
 */
void TIM_Init(){
    TIM_ClockConfigTypeDef sClockSourceConfig;
    TIM_MasterConfigTypeDef sMasterConfig;
    TIM_OC_InitTypeDef sConfigOC;

    __HAL_RCC_TIM3_CLK_ENABLE();

    /* Set up the clock/prescaler to overflow in ~1s */
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 100;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 64000;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.RepetitionCounter = 0x0;
    HAL_TIM_Base_Init(&htim3);

    /* Internal clock source */
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig);

    /* Clock synchronisation/master/slave config - all disabled */
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig);

    /* Set up the output compare value for each channel. I have just hard coded these as an example */
    sConfigOC.OCMode = TIM_OCMODE_ACTIVE;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.Pulse = 10000;
    HAL_TIM_OC_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1);
    sConfigOC.Pulse = 35000;
    HAL_TIM_OC_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2);
    sConfigOC.Pulse = 50000;
    HAL_TIM_OC_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3);

    /*
     * Enable the interrupt for TIM3
     * Note that this enables *all* interrupts on TIM3 - interrupt code needs to determine the source
     * The interrupt function must be called TIM3_IRQHandler()
     */
    HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);
}


/*
 * TIM3 interrupt handler.
 * This handles *all* interrupts form TIM3, so we need to find out what actually triggered this one
 * To do that just call HAL_TIM_IRQHandler with a reference to the global timer3 object we used to initialise the timer
 * It will then automatically call the correct interrupt handler for whatever interrupt was actually generated
 *   (note this requires specifically named interrupt handlers. See the code for HAL_TIM_IRQHandler for the names)
 */
void TIM3_IRQHandler(){
    HAL_TIM_IRQHandler(&htim3);
}

/*
 * Interrupt handler for OC interrupt
 * Called automatically by HAL_TIM_IRQHandler
 * This is common to all timer OC interrupts, so it needs to check which timer triggered it
 *   (ie. check htim->Instance)
 */
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim){
	/* Check which timer caused the interrupt */
	if(htim->Instance == TIM3){
		/* Toggle LEDs depending on which channel triggered this interrupt */
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1){
			HAL_GPIO_TogglePin(LED3_GPIO_PORT, LED3_PIN);
		}else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2){
			HAL_GPIO_TogglePin(LED4_GPIO_PORT, LED4_PIN);
		}else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3){
			HAL_GPIO_TogglePin(LED5_GPIO_PORT, LED5_PIN);
		}
	}
}

/*
 * Interrupt handler for timer overflow
 * Called automatically by HAL_TIM_IRQHandler
 * This is common to all timer overflow interrupts, so it needs to check which timer triggered it
 *   (ie. check htim->Instance)
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	/* Check which timer caused the interrupt */
	if(htim->Instance == TIM3){
		HAL_GPIO_TogglePin(LED6_GPIO_PORT, LED6_PIN);
	}
}
