/***************************************************************************//**
  @file     event_handler.h
  @brief    Interface of event handler
  @author   Tobias Demeco
 ******************************************************************************/

#ifndef _EVENTH_H_
#define _EVENTH_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "header/Node-RED/ESP8266_UART.h"
/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define SPI_BITR 1
#define SPI_BITL 2
#define SPI_BITP 3

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
typedef enum {
    NO_EVENT,
    PRESSED_L,
    PRESSED_R,
    PRESSED_P,
    DATA_CHANGE,
    NODE_UPDATE,
    LED_UPDATE
} System_Events;

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initializes event handler
 */
void event_handler_init(void);

/**
 * @brief Gets next event from queue, then deletes it
 */
System_Events get_next_event(void);


/*******************************************************************************
 ******************************************************************************/

#endif // _EVENTH_H_