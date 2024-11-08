/***************************************************************************//**
  @file     SPI_joystick.h
  @brief    +Descripcion del archivo+
  @author   Gian
 ******************************************************************************/

/*#ifndef _SPI_JOYSTICK_H_
#define _SPI_JOYSTICK_H_
*/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
/*#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
*/

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
/*#define MAX_SPI_JOYSTICK_QUEUE			50
#define SPI__JOYSTICK_REFRESH_PERIOD	100
*/

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
/*typedef enum{
	NO_JOYSTICK_EVENT,
	INVALID_SPI_JOYSTICK_EVENT,
	SPI_JOYSTICK_L_PRESS,
	SPI_JOYSTICK_R_PRESS,
	SPI_JOYSTICK_P_PRESS,
	END_OF_SPI_JOYSTICK_QUEUE
}SPI_Joystick_Events;

typedef uint8_t joystickID;
*/

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

// +ej: extern unsigned int anio_actual;+


/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initializes the joystick driver
 */
//void spiJoystickInit();


/**
 * @brief create a new encoder structure
 * @param id: The SPI use by the new joystick
 * @return ID for a new encoder, will return -1 if it couldn't create new joystick;
 */
//joystickID getSpiJoystickID(void);


/**
 * @brief Read the oldest event for a given joystick, the user must go through the queue if he needs it, until END_OF_QUEUE reached
 * @param id: the id of the given joystick
 * @return oldest event in the event queue, will return INVALID if the id corresponds to a disabled or invalid joystick
 */
//const SPI_Joystick_Events getSpiJoystickEvent(joystickID id);

/**
 * @brief Disables a given encoder
 * @param id: the id of the given encoder
 */
//void disableSpiJoystick(joystickID id);


/**
 * @brief Enables a given joystick
 * @param id: the id of the given joystick
 */
//void enableSpiJoystick(joystickID id);


/**
 * @brief Informs on wheater or not a given joystick is enabled
 * @param id: the id of the given joystick
 * @return TRUE if a joystick is enabled, FALSE if the joystick deesn't exist or is disabled
 */
//bool isSpiJoystickEnabled(joystickID id);


/*******************************************************************************
 ******************************************************************************/

//#endif // _TEMPLATE_H_