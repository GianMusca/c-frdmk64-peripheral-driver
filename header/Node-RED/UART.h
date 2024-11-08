/***************************************************************************//**
  @file     UART.h
  @brief    UART communication service
  @author   Ale
 ******************************************************************************/

#ifndef _UART_H_
#define _UART_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef enum {
	UART_0,
	UART_1,
	UART_2,
	UART_3,
	UART_4,
	UART_5
} UART_id;				//There can be up to 5 UART communications

typedef enum{
	UART_BLOCKING,
	UART_NON_BLOCKING
} UART_mode;

typedef enum{
	UART_NO_PARITY,
	UART_EVEN,
	UART_ODD
} UART_parity;

typedef enum{
	NBITS_8 = 8,
	NBITS_9 = 9
} UART_nbits;

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initializes UART communication protocol
 * @param id: ID of the UART used
 * @param baud_rate: baud_rate to use in the communication, agreed by both TX and RX
 * @param mode:
 * @param parity: type of parity, UART_NO_PARITY, UART_EVEN, UART_ODD
 * @param nbits: amount of bits NBITS_8, NBITS_9
 * @return
 */
void UART_init (UART_id id, uint32_t baud_rate, UART_mode mode, UART_parity parity, UART_nbits nbits);

/**
 * @brief checks the state of reception of data
 * @param id: ID of the UART to check
 * @return weather or not the UART is receiving data
 */
bool UART_is_new_info (UART_id id);

/**
 * @brief Used to know how many bytes of data are stored in the RX buffer
 * @param id: ID of the UART to check
 * @return amount of bytes stored in the RX buffer
 */
uint32_t UART_get_msg_length (UART_id id);

/**
 * @brief Transfer data from RX buffer to an external buffer
 * @param id: ID of the UART to check
 * @param buff: buffer to transfer the received data
 * @param length: amount of bytes to transfer
 * @return
 */
void UART_read (UART_id id, char* buff, uint32_t length);

/**
 * @brief checks the state of transmission of data
 * @param id: ID of the UART to check
 * @return weather or not the UART is sending data
 */
bool UART_is_TX_done (UART_id id);

/**
 * @brief Send data through the chosen UART
 * @param id: ID of the UART to use
 * @param msg: buffer where the message is stored
 * @param length: amount of bytes to transfer (transmission starts at msg[0])
 * @return if the msg was successfully stored to be transmitted
 */
bool UART_write(UART_id id, const char* msg, uint32_t length);


/*******************************************************************************
 ******************************************************************************/

#endif // _UART_H_
