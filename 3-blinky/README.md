# Control LED brightness with PWM

Uses the PWM output from TIM3 to pulse LED brightness.
3 LEDs are connected to 3 PWM output channels (4th is unused).
The main loop then ramps the duty cycle of each channel/LED separately so that they pulse at different rates.

Note: PWM outputs are hard wired to certain pins in the chip, and so cannot use the onboard LEDs.
You need to wire 3 LEDs + current limiting resistors to:
 - PORTA_6
 - PORTA_7
 - PORTB_0
 
This example is slightly more involved and uses an MSP function call (MCU Support Package - ie. MCU specific initialisation code).
The PWM init code sets up the timer and output compare registers which is all common across all of the stm32 controllers.
But the output pin mapping for different peripherals can change for different controllers. 
So the idea is to move that code out into a separate MSP function call so that when changing controllers, only the MSP code needs to be changed.
 
In this case, the standard HAL function HAL_TIM_PWM_Init() looks for a function (which I have to define) called HAL_TIM_Base_MspInit().
This is defined in a separate c-file stm32f4x_hal_msp_template.c

This works by overriding a default MspInit definition included in the libary code using weak linkage (https://blog.feabhas.com/2013/01/weak-linkage-in-c-programming/)
Basically, the default definition included in the libary is defined as __weak which tells the linker that if another definition is provided then use that instead.
Here is the full definition from the HAL libary code (HAL_Driver/Src/stm32f4xx_hal_tim.c)
```c
__weak void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(htim);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the HAL_TIM_Base_MspInit could be implemented in the user file
   */
}
```