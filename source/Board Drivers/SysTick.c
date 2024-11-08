/***************************************************************************//**
  @file     App.c
  @brief    Application functions
  @author   AleX
 ******************************************************************************/

// VERSION UN SOLO CLOCK!!!
// VERSION 1MS

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "SysTick.h"
#include <stddef.h>
#include <stdint.h>
#include "MK64F12.h"

//#include "gpio.h"
//#include "board.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define SYSTICK_ISR_FREQUENCY_HZ 	100000000UL				//100MHz
#define SYSTICK_CALL_PERIOD_US		1000UL					//Want a period of 1000us between systicks
#define LOADS 						(SYSTICK_CALL_PERIOD_US*(SYSTICK_ISR_FREQUENCY_HZ/1000000UL))
#define MS2COUNTS(periodo) (periodo/1)					//1.22 is an empiric value, based on measurements
#define MAX_CALLBACKS				16

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/
__ISR__ SysTick_Handler(void);
/*******************************************************************************
                        GLOBAL VARIABLES
 ******************************************************************************/
int flag_systick;
static systick_callback_t timer_callback = NULL;
/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/**
 * @brief Initialize SysTic driver
 * @param irqs_callback, array of Callback interrupts to be called every SysTick
 * @param n, amount of elements in irqs_callback[]
 * @return Initialization and registration succeed
 */
bool SysTick_Init (systick_callback_t irqs_callback){
							//See hardware.h for definition
	bool res = false;											//Initial return is false, meaning failure to initialize SysTick

	if (irqs_callback != NULL){

		timer_callback = *irqs_callback;

		SysTick->CTRL = 0x00; 												//Enable SysTick interrupt
		SysTick->LOAD = LOADS - 1;
		SysTick->VAL=0x00;
		SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk| SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk;		//See definitions in core_cm4.h, lines 767~796

		NVIC_EnableIRQ(SysTick_IRQn);

		res = true;
	}

	return res;

}

/**
 * @brief function called each SysTick
 */

// TODO probar con la digi el tiempo q tarda la interrupcion
__ISR__ SysTick_Handler(void){
	//gpioWrite(PIN_LED_GREEN,0);

	if(timer_callback != NULL)
		timer_callback();

	//gpioWrite(PIN_LED_GREEN,1);
}
