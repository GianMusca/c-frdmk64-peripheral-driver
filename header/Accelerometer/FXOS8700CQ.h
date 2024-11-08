/***************************************************************************//**
  @file     FXOS8700CQ.h
  @brief    Simple GPIO Pin services, similar to Arduino
  @author   Matías Tripoli
 ******************************************************************************/

#ifndef FXOS8700CQ_H_
#define FXOS8700CQ_H_

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

typedef struct{
	int16_t x;
	int16_t y;
	int16_t z;
}Acceleration_Type;

typedef enum {FXOS8700CQ_INIT, FXOS8700CQ_READY, FXOS8700CQ_ERROR} FXOS8700CQ_State_Type;

typedef  struct{
	FXOS8700CQ_State_Type state;
	Acceleration_Type accel[2];
	Acceleration_Type accel_buffer[2];
	uint8_t I2C_read_queue[2];
	uint8_t I2C_write_queue[2];
	uint32_t step;
	uint8_t index;
	bool done;
}FXOS8700CQ_Type;

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

void FXOS8700CQ_Init(void);		// FXOS8700CQ Init
bool FXOS8700CQ_Done(void);		// FXOS8700CQ done getting data
bool FXOS8700CQ_Is_Running(void);		// FXOS8700CQ is getting data
Acceleration_Type FXOS8700CQ_get_Acceleration(void);		// get Acceleration vector
int16_t FXOS8700CQ_get_Accel_x(void);		// get Acceleration x-component
int16_t FXOS8700CQ_get_Accel_y(void);		// get Acceleration y-component
int16_t FXOS8700CQ_get_Accel_z(void);		// get Acceleration z-component

/*******************************************************************************
 ******************************************************************************/

#endif /* FXOS8700CQ_H_ */
