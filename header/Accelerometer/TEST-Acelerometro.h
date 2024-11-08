/*
 * Acelerometro.h
 *
 *  Created on: 31/10/2016
 *      Author: USUARO
 */

#ifndef SOURCES_ACELEROMETRO_H_
#define SOURCES_ACELEROMETRO_H_

////////#include "I2C.h"
/*****************************************Definiciones segun hoja de datos*******************************************/
// FXOS8700CQ I2C Direcciones
#define FXOS8700CQ_SLAVE_ADDR0 (0x1E<<1) // with pins SA0=0, SA1=0
#define FXOS8700CQ_SLAVE_ADDR1 (0x1D<<1) // with pins SA0=1, SA1=0
#define FXOS8700CQ_SLAVE_ADDR2 (0x1C<<1) // with pins SA0=0, SA1=1
#define FXOS8700CQ_SLAVE_ADDR3 (0x1F<<1) // with pins SA0=1, SA1=1

// FXOS8700CQ internal register addresses
#define FXOS8700Q_STATUS 0x00
#define FXOS8700Q_OUT_X_MSB 0x01
#define FXOS8700Q_OUT_Y_MSB 0x03
#define FXOS8700Q_OUT_Z_MSB 0x05
#define FXOS8700Q_M_OUT_X_MSB 0x33
#define FXOS8700Q_M_OUT_Y_MSB 0x35
#define FXOS8700Q_M_OUT_Z_MSB 0x37
#define FXOS8700Q_WHOAMI 0x0D
#define FXOS8700Q_XYZ_DATA_CFG 0x0E
#define FXOS8700Q_CTRL_REG1 0x2A
#define FXOS8700Q_M_CTRL_REG1 0x5B
#define FXOS8700Q_M_CTRL_REG2 0x5C
#define FXOS8700Q_M_CTRL_REG3 0x5D
#define FXOS8700Q_WHOAMI_VAL 0xC7



typedef struct{
    int16_t accel_x;
	int16_t accel_y;
	int16_t accel_z;
} accel_t;


/* ***************************************Funciones************************************************/

void Accelerometro_Init(void);
void Acelerometro_enable(void);
void Acelerometro_disable(void);

int16_t Acelerometro_getXint(void);
int16_t Acelerometro_getYint(void);
int16_t Acelerometro_getZint(void);

accel_t get_accelerations(void);


#endif
