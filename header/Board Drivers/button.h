/***************************************************************************//**
  @file     button.h
  @brief    Button driver. Advance implementation
  @author   Nicolás Magliola
 ******************************************************************************/

#ifndef _BUTTON_H_
#define _BUTTON_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define MAX_BUFF			200

#define BUTTON_REFRESH_PERIOD	10			//10mseg between each buttonUpdate
#define LKP_THRESHOLD			50			//50*10mseg for an LKP
#define SKP_THRESHOLD			10			//10*10mseg for a SKP
#define TYPEMATIC_THRESHOLD		50			//50*10mseg for start of TYPEMATIC
#define TYPEMATIC_PERIOD		3			//each 3*10mseg a new PRESS will be triggered
/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
typedef int buttonID;
typedef enum{
	NO_BUTTON_EVENT,
	INVALID_BUTTON_EVENT,
	BUTTON_PRESS,
	BUTTON_RELEASE,
	BUTTON_LKP,
	BUTTON_SKP,
	BUTTON_TYPEMATIC,
	END_OF_BUTTON_QUEUE
}buttonEvents;
/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/
/**
 * @brief Initializes the button driver
 */
void buttonInit(void);
/**
 * @brief create a new button structure
 * @param pin: the pin of the new button
 * @param active: whether its ACTIVE_HIGH or ACTIVE_LOW
 * @return ID for a new button, will return -1 if it couldn't create new button;
 */
buttonID getButtonID(pin_t pin, uint8_t mode, bool active);
/**
 * @brief Read the oldest event for a given button, then deletes it from the event queue
 * @param id: the id of the given button
 * @return oldest event in the event queue, will return INVALID if the id corresponds to a disabled or invalid button
 */
const buttonEvents *buttonGetEvent(buttonID id);

/**
 * @brief Reads the input of a button
 * @param id: the id of the given button
 * @return either TRUE or FALSE
 */
bool buttonRead(buttonID id);

/**
 * @brief Returns time that the given button has been pressed, in mseg, in increments of BUTTON_REFRESH_PERIOD
 * @param id: the id of the given button
 * @return time in mseg
 */
uint32_t getTimePressed(buttonID);

/**
 * @brief Attaches interrupt to a button, IGNORES ENABLE OF BUTTON AFTER THE FACT
 * @param id: the id of the given button
 * @param IRQMode: see IRQC modes enum definition at top of gpio.h
 * @param IRQFunction: function to call at pin interrupt triggered
 * @return validation of configuration
 */
bool buttonSetInterrupt(buttonID id, uint8_t IRQMode, pinIRQFunction_t IRQFun);

/**
 * @brief Disables a given button
 * @param id: the id of the given button
 */
void disableButton(buttonID id);
/**
 * @brief Enables a given button
 * @param id: the id of the given button
 */
void enableButton(buttonID id);
/**
 * @brief Informs on wheater or not a given button is enabled
 * @param id: the id of the given button
 * @return TRUE if a button is enabled, FALSE if the button deesn't exist or is disabled
 */
bool isButtonEnabled(buttonID id);
/*******************************************************************************
 ******************************************************************************/

#endif // _BUTTON_H_

