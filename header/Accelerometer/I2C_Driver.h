/***************************************************************************//**
  @file     I2C_Driver.h
  @brief    Driver for I2C
  @author   Matías Tripoli
 ******************************************************************************/

#ifndef _I2C_DRIVER_H_
#define _I2C_DRIVER_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include "MK64F12.h"
#include "hardware.h"
#include "gpio.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

// Kinetis K64 Datasheet - 5.1 K64 Signal Multiplexing and Pin Assignments
#define I2C0_SCL	PORTNUM2PIN(PE, 24)		// PTE24
#define I2C0_SDA	PORTNUM2PIN(PE, 25)		// PTE25
#define I2C1_SCL	PORTNUM2PIN(PC, 10)		// PTC10
#define I2C1_SDA	PORTNUM2PIN(PC, 11)		// PTC11
#define I2C2_SCL	PORTNUM2PIN(PA, 12)		// PTA12
#define I2C2_SDA	PORTNUM2PIN(PA, 11)		// PTA11

#define I2C0_ALT 5		// PTE24: ALT5 & PTE25: ALT5
#define I2C1_ALT 2		// PTC10: ALT2 & PTC11: ALT2
#define I2C2_ALT 5		// PTA12: ALT5 & PTA11: ALT5

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef enum {I2C_IDLE, I2C_COM_CONFIG, I2C_END, I2C_ERROR} I2C_State_Type;

typedef void (*I2C_Callback_Type)(void);

typedef struct {
	I2C_State_Type	state;
	bool       	readnotwrite;
	uint32_t   	slave_address;
	uint8_t*  	write_queue;
	uint8_t*  	read_queue;
	uint8_t		write_queue_size;
	uint8_t     read_queue_size;
	uint8_t     writing_byte_counter;
	uint8_t     reading_byte_counter;
	I2C_Callback_Type	end_routine;
} I2C_Communication_Type;

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/


/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

void I2C_Init(void);
void I2C_Communication_Config(bool rnotw, uint32_t s_addr, uint8_t* w_q, uint8_t* r_q, uint8_t w_q_s, uint8_t r_q_s);
I2C_State_Type I2C_get_State(void);
void I2C_End_Routine(I2C_Callback_Type callback);

#endif // _I2C_DRIVER_H_
