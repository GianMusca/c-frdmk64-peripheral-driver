/***************************************************************************//**
  @file     +SPI.h
  @brief    +Descripcion del archivo+
  @author   Gian
 ******************************************************************************/

#ifndef _SPI_H_
#define _SPI_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/



/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
typedef enum{
       SPI_0,
       SPI_1,
       SPI_2,
	   MAX_SPI_ID
} SPI_id;

typedef enum{
       SLAVE,
       MASTER
} SPI_mode_t;

typedef enum{
       SCK_LOW,
       SCK_HIGH
} SPI_clock_polarity_t;

typedef enum{
       CPHA_LOW,
       CPHA_HIGH
} SPI_clock_phase_t;

typedef enum{
       MSB,
       LSB
} SPI_endiannes;

typedef enum{
       INACTIVE_L,
       INACTIVE_H
} SPI_SS_polarity;

typedef enum{
       SLAVE_0 = 0b000001,
	   SLAVE_1 = 0b000010,
	   SLAVE_2 = 0b000100,
	   SLAVE_3 = 0b001000,
	   SLAVE_4 = 0b010000,
	   SLAVE_5 = 0b100000
} SPI_slave;

typedef struct{
	uint32_t         	baudrate;
	uint8_t           	frame_size;
	SPI_mode_t           	mode   			: 1;
	SPI_SS_polarity     pcs_polarity		: 1;
	SPI_clock_polarity_t	sck_polarity    : 1;
	SPI_clock_phase_t    sck_phase       	: 1;
	SPI_endiannes       endiannes      		: 1;
} SPI_cfg_t;


/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

// +ej: extern unsigned int anio_actual;+


/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief TODO: completar descripcion
 * @param param1 Descripcion parametro 1
 * @param param2 Descripcion parametro 2
 * @return Descripcion valor que devuelve
*/

void SPI_init(SPI_id id, SPI_cfg_t);

void SPI_read(SPI_id id, uint16_t* buff);
bool SPI_read_done(SPI_id id);
bool SPI_RX_full(SPI_id id);
bool SPI_RX_empty(SPI_id id);

bool SPI_write(SPI_id id, SPI_slave slave, const uint16_t* msg, uint32_t length);
bool SPI_write_done(SPI_id id);
bool SPI_TX_full(SPI_id id);
bool SPI_TX_empty(SPI_id id);



/*******************************************************************************
 ******************************************************************************/

#endif // _SPI_H_
