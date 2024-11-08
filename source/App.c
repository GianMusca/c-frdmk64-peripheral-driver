/***************************************************************************//**
  @file     App.c
  @brief    Application
  @author   Tobias Demeco
 ******************************************************************************/

#define MYDEBUG false

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "header/FSM/FSM.h"
#include "header/FSM/FSMTable.h"

#include "header/Event Handler/event_handler.h"
#include "header/Accelerometer/Acelerometro.h"
#include "header/LED Matrix/LED_matrix_app.h"

#include "header/Board Drivers/timer.h"



/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

State_Type *p2state;					//state pointer

static void update_position(void);

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/* Función que se llama 1 vez, al comienzo del programa */
void App_Init (void)
{
	event_handler_init();				//initialize event handler (it initializes timer, spi joystick, esp uart, accelerometer and potenciometer)

	LED_matrix_app_init();				//initialize led matrix

	p2state = horizontal;				//init program with horizontal joystick movement
}

/* Función que se llama constantemente en un ciclo infinito */
void App_Run (void)
{
    System_Events event = get_next_event();
	  p2state = fsm(p2state, event);
}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/*******************************************************************************
 ******************************************************************************/
