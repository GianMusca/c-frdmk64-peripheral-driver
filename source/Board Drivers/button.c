/***************************************************************************//**
  @file     encoder.c
  @brief    Application functions
  @author   Ale
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "header/board.h"
#include "gpio.h"
#include "button.h"
#include "timer.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define MAX_BUTTONS		10

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
//Structure for any button
typedef struct {
	buttonID id;
	pin_t pin;				//Where it is connected
	bool active;			//Active LOW or Active High
	bool enable;			//It can be declared and left momentarily disabled
	bool flag;				//Flag to detect changes
	bool pressBuffer[MAX_BUFF];
} button_t;
/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL LEVEL SCOPE
 ******************************************************************************/

static button_t buttonArray[MAX_BUTTONS];				//Stores info on all possible buttons
static buttonEvents eventBuffer[MAX_BUTTONS][MAX_BUFF];	//An event queue for every button
static uint32_t pressIndex[MAX_BUFF] = {1};				//Index for how many button ticks passed since last buttonGetEvent call
static const buttonEvents invalidPointer = INVALID_BUTTON_EVENT;
static uint32_t timeCounter[MAX_BUTTONS] = {0};
/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

bool isValidButtonID(buttonID id);
void buttonsUpdate(void);
bool isPressed(bool read, bool active);
/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
/**
 * @brief Configures clock gating mode for a preipheral
 * @param pin: the pin whose clock gating mode you wish to set (according PORTNUM2PIN)
 * @param mode GC_ON, GC_OFF.
 */
void buttonInit(void){
	static bool initialized =false;
	if(initialized)
		return;
	uint8_t i;
	timerInit();			//Just in case our user hasn't already initialized the timer driver
	tim_id_t buttonTimer = timerGetId();
	timerStart(buttonTimer, TIMER_MS2TICKS(BUTTON_REFRESH_PERIOD), TIM_MODE_PERIODIC, buttonsUpdate);	//Polling of sorts
	for(i=0; i<MAX_BUTTONS; i++)
		buttonArray[i].id = -1;
}

const buttonEvents *buttonGetEvent(buttonID id){
	int i = 0;
	int eventCounter = 0;
	if(!isValidButtonID(id))
		return &invalidPointer;
	if (buttonArray[id].flag){							 								//if the buttons has an event queue
		for (i = 0; i < pressIndex[id]; i++){											//for every press on the press Buffer
			if (buttonArray[id].pressBuffer[i + 1] == buttonArray[id].pressBuffer[i]){ 	//If two subsequent presses are equal, it could be an SKP, LKP or Typematic
				if (buttonArray[id].pressBuffer[i + 1] == true){						//If they are equal and pressed
					timeCounter[id] = timeCounter[id] + 1;								//Increase time counter
					if (((timeCounter[id] > TYPEMATIC_THRESHOLD) && (timeCounter[id] % TYPEMATIC_PERIOD))){ //Every TYPEMATIC_PERIOD, a new PRESS will be registered on the queue
						eventBuffer[id][eventCounter++] = BUTTON_PRESS; 						//Typematic equivalent
					}
					if ((timeCounter[id] > LKP_THRESHOLD) && (timeCounter[id] < LKP_THRESHOLD + 2))//If enough time has passed, a new LKP event will be registered
						eventBuffer[id][eventCounter++] = BUTTON_LKP;
				}
				else																	//If two subsequent presses are equal and unpressed, timer restarts
					timeCounter[id] = 0;
			}
			else{																		//If a sequence of two is different, it could be either PRESS or RELEASE
				if (buttonArray[id].pressBuffer[i + 1] == true)							//If the latest is pressed, the event is a PRESS
					eventBuffer[id][eventCounter++] = BUTTON_PRESS;
				else																	//If the latest is unpressed, the event is RELEASE
					eventBuffer[id][eventCounter++] = BUTTON_RELEASE;
				if ((timeCounter[id] > SKP_THRESHOLD) && (timeCounter[id] < LKP_THRESHOLD) && (buttonArray[id].pressBuffer[i + 1] == false))
					eventBuffer[id][eventCounter++] = BUTTON_SKP; 	//If timer has started, and is longer than a PRESS, but isn't an LKP, it is an SKP
				timeCounter[id] = 0; 													//And the timer is reset
			}
		}
		eventBuffer[id][eventCounter] = END_OF_BUTTON_QUEUE; 									//When the sequence is finished, END_OF_QUEUE
	}
	else
		eventBuffer[id][eventCounter] = END_OF_BUTTON_QUEUE; 									//When there was no sequence, END_OF_QUEUE
	buttonArray[id].pressBuffer[0] = buttonArray[id].pressBuffer[i];					//Put last value as the first of a new sequence
	buttonArray[id].flag = false;														//The whole queue was serviced, so we don't need the flag
	const buttonEvents * ret = (const buttonEvents *)&(eventBuffer[id][0]); 								//Convert event matrix to event queue
	return ret;
}

/**
 * @brief create a new button structure
 * @param pin: the pin of the new button
 * @param acive: wheather its ACTIVE_HIGH or ACTIVE_LOW
 * @return ID for a new button, will return -1 if it couldn't create new button;
 */
buttonID getButtonID(pin_t pin, uint8_t mode, bool active){
	buttonID id = -1;
	uint8_t i;
	for(i=0; i<MAX_BUTTONS; i++)
		if(buttonArray[i].id == -1){
			id = i;
			buttonArray[i].id = i;
			buttonArray[i].pin = pin;
			buttonArray[i].active = active;
			buttonArray[i].enable = true;
			buttonArray[i].flag = false;
			uint8_t j;
			gpioMode(pin, mode, DSE_DEFAULT, ODE_DEFAULT, SRE_DEFAULT, PFE_DEFAULT, DFE_DEFAULT, 0);
			for(j=0; j< MAX_BUFF; j++)
				buttonArray[i].pressBuffer[j] = NO_BUTTON_EVENT;
			break;
		}
	return id;
}

/**
 * @brief Returns time that the given button has been pressed, in mseg, in increments of BUTTON_REFRESH_PERIOD
 * @param id: the id of the given button
 * @return time in mseg
 */
uint32_t getTimePressed(buttonID id){
	uint32_t timePressed = 0;
	if(isValidButtonID(id))
		timePressed = timeCounter[id]*BUTTON_REFRESH_PERIOD;
	return timePressed;
}


/**
 * @brief Reads the input of a button
 * @param id: the id of the given button
 * @return either TRUE or FALSE, if button is disabled, it will always return FALSE
 */
bool buttonRead(buttonID id){
	bool ret = false;
	bool read;
	bool active;
	uint8_t i;
	if(isValidButtonID(id)){
		for(i=0; i<MAX_BUTTONS; i++){
			if(buttonArray[i].id == id && buttonArray[i].enable == true){
				read = gpioRead(buttonArray[i].pin);
				active = buttonArray[i].active;
				ret = isPressed(read, active);
				break;
			}
		}
	}
	return ret;
}

/**
 * @brief Disables a given button
 * @param id: the id of the given button
 */
void disableButton(buttonID id){
	if(isValidButtonID(id))
		buttonArray[id].enable = false;
}
/**
 * @brief Enables a given button
 * @param id: the id of the given button
 */
void enableButton(buttonID id){
	if(isValidButtonID(id))
		buttonArray[id].enable = true;
}
/**
 * @brief Informs on wheater or not a given button is enabled
 * @param id: the id of the given button
 * @return TRUE if a button is enabled, FALSE if the button deesn't exist or is disabled
 */
bool isButtonEnabled(buttonID id){
	bool ret = false;
	if(isValidButtonID(id))
		ret = buttonArray[id].enable;
	return ret;
}

/*******************************************************************************
 *******************************************************************************
                        PRIVATE FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

bool isPressed(bool read, bool active){
	bool ret = false;
	if(read == active)
		ret = true;
	return ret;
}

/**
 * @brief validates button ID
 * @param id: the id of the given button
 * @return either TRUE or FALSE
 */
bool isValidButtonID(buttonID id){
	bool valid = false;
	uint8_t i;
	for(i=0; i<MAX_BUTTONS; i++){
		if(buttonArray[i].id == id)
			valid = true;
	}
	return valid;
}


/*******************************************************************************
                        INTERRUPT ZONE
 ******************************************************************************/
/**
 * @brief Attaches interrupt to a button, IGNORES ENABLE OF BUTTON AFTER THE FACT
 * @param id: the id of the given button
 * @param IRQMode: see IRQC modes enum definition at top of gpio.h
 * @param IRQFunction: function to call at pin interrupt triggered
 * @return validation of config
 */
bool buttonSetInterrupt(buttonID id, uint8_t IRQMode, pinIRQFunction_t IRQFun){
	bool ret = false;
	if(isValidButtonID(id))
		ret = gpioIRQ (buttonArray[id].pin, IRQMode, IRQFun);
	return ret;
}


/*******************************************************************************
                        TIMER ZONE
 ******************************************************************************/
/**
 * @brief Updates buttons events with timers
 */
void buttonsUpdate(void){
	uint8_t i;
	for(i = 0; i< MAX_BUTTONS; i++){			//For every enabled button on the array
		if(buttonArray[i].enable){
			if(buttonArray[i].flag == false){			//If there wasn't an active event ongoing
				buttonArray[i].flag = true;				//Now there is
				buttonArray[i].pressBuffer[1] = buttonRead(buttonArray[i].id);	//pressBuffer[0] contains the last value of the previous event
																				//pressBuffer[1] contains the now most recent value of button
				pressIndex[i] = 1;				//Now it's been 1 button tick since last buttonRead, meaning reset the press counter
			}
			else{								//If there was an active press queue
				if(pressIndex[i] < MAX_BUFF-1)	//If there is room on the buffer
					pressIndex[i]++;			//Increment the amount of buttonReads since last buttonGetEvent
				else
					pressIndex[i] = 0;			//Otherwise reset the circular buffer
				buttonArray[i].pressBuffer[pressIndex[i]] = buttonRead(buttonArray[i].id);	//load the most recent buttonRead
			}
		}
	}
	return;
}
