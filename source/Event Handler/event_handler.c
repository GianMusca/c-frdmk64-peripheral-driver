/***************************************************************************//**
  @file     event_handler.c
  @brief    Handler of system events
  @author   Tobias Demeco
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "header/Event Handler/event_handler.h"

#include "header/Joystick/SPI_joystick.h"
#include "header/Accelerometer/FXOS8700CQ.h"
#include "header/Potenciometer/potenciometer.h"

#include "header/Board Drivers/timer.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define MAX_EVENT_NUMBER 50

static System_Events eventQueue[MAX_EVENT_NUMBER];
static int beginEvent = 0;
static int endEvent = 0;

static joystickID joy_id = 0;

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

void update_events(void);

void add_to_queue(System_Events newEvent);

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void event_handler_init(void){												//INITIALIZATION OF EVENT HANDLER
	beginEvent = 0;															//starts the queue
	endEvent = 0;

	spiJoystickInit(SPI_BITR, SPI_BITL, SPI_BITP);							//initializes SPI joystick (change constants in event_handler.h acording to connection)
	joy_id = getSpiJoystickID();
	enableSpiJoystick(joy_id);

	pote_init();															//initializes potenciometer for reading

	FXOS8700CQ_Init();														//initializes accelerometer readings

	ESP8266_UART_init();													//initializes UART comms

	timerInit();															//initializes timers driver
	tim_id_t timer_id = timerGetId();
	timerStart(timer_id, 5, TIM_MODE_PERIODIC, update_events);				//initializes a periodic timer to check for events (and update node-RED ui)
}

System_Events get_next_event(void){											//NEXT EVENT GETTER
	if (beginEvent == endEvent)												//if there is no event, return NO_EVENT
		return NO_EVENT;
	else{																	//if there is an event, save it and increment the index of the first event
		System_Events event = eventQueue[beginEvent++];
		if(beginEvent >= MAX_EVENT_NUMBER)									//if we went over the limit, start over (cyclic buffer)
			beginEvent = 0;
		return event;
	}
}

/*******************************************************************************
 *******************************************************************************
                        PRIVATE FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void update_events(void){						//EVENT UPDATER (called every 5ms)
	
	static int counter = 0;
	if(counter++ == 1)							//Every 10ms we update led position and Node-RED ui
	{
		add_to_queue(LED_UPDATE);					
		add_to_queue(NODE_UPDATE);			
		counter = 0;									
	}

	SPI_Joystick_Events nextJoyEvent = getSpiJoystickEvent(joy_id);			//Get the last joystick event	
	switch(nextJoyEvent)
	{
		case NO_JOYSTICK_EVENT:
			break;
		case INVALID_SPI_JOYSTICK_EVENT:
			//error function?
			break;
		case SPI_JOYSTICK_L_PRESS:
			add_to_queue(PRESSED_L);
			break;
		case SPI_JOYSTICK_R_PRESS:
			add_to_queue(PRESSED_R);
			break;
		case SPI_JOYSTICK_P_PRESS:
			add_to_queue(PRESSED_P);
			break;
		default:
			break;
	}

	if (new_ESP_data_available())					//Check for pending messages from ESP
	{
		add_to_queue(DATA_CHANGE);
	}
}

void add_to_queue(System_Events newEvent){
	eventQueue[endEvent++] = newEvent;
	if(endEvent >= MAX_EVENT_NUMBER)
		endEvent = 0;
	if (beginEvent == endEvent){
		beginEvent++;
		if(beginEvent >= MAX_EVENT_NUMBER)
			beginEvent = 0;
	}
}

/*******************************************************************************
                        INTERRUPTS ZONE
 ******************************************************************************/
