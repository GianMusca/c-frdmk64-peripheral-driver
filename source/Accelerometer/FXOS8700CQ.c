/***************************************************************************//**
  @file     FXOS8700CQ.c
  @brief    Application functions
  @author   Matías Tripoli
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "I2C_Driver.h"
#include "FXOS8700CQ.h"
#include "timer.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

// FXOS8700CQ I2C address
#define FXOS8700CQ_SLAVE_ADDR 0x1D // with pins SA0=0, SA1=0

// FXOS8700CQ internal register addresses
#define FXOS8700CQ_STATUS	0x00
#define OUT_X_MSB	0x01
#define OUT_X_LSB	0x02
#define OUT_Y_MSB	0x03
#define OUT_Y_LSB	0x04
#define OUT_Z_MSB	0x05
#define OUT_Z_LSB	0x06
#define FXOS8700CQ_WHOAMI	0x0D
#define FXOS8700CQ_XYZ_DATA_CFG	0x0E
#define FXOS8700CQ_CTRL_REG1	0x2A
#define FXOS8700CQ_M_CTRL_REG1	0x5B
#define FXOS8700CQ_M_CTRL_REG2	0x5C
#define FXOS8700CQ_WHOAMI_VAL	0xC7

#define FXOS8700CQ_TICKS 50

/*******************************************************************************
 * CONSTANTS AND GLOBAL VARIABLES
 ******************************************************************************/

static FXOS8700CQ_Type module;

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

void  FXOS8700CQ_ISR(void);
static void FXOS8700CQ_Read(uint8_t reg);
static void FXOS8700CQ_Write(uint8_t reg, uint8_t data);
static void FXOS8700CQ_Config(bool reset);
static void FXOS8700CQ_Run(bool reset);
static void FXOS8700CQ_End(void);

/*******************************************************************************
 *******************************************************************************
 *                      GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void FXOS8700CQ_Init(void){
	//FXOS8700CQ Config
	module.state = FXOS8700CQ_INIT;

	// I2C
	I2C_Init();		// Init I2C
	I2C_End_Routine(FXOS8700CQ_End);

	// Timer
	timerInit();		// Init Timer
	tim_id_t FXOS8700CQ_timer = timerGetId();		// get Timer Id
	timerStart(FXOS8700CQ_timer, FXOS8700CQ_TICKS,TIM_MODE_PERIODIC, FXOS8700CQ_ISR);		// Start Timer
}

bool FXOS8700CQ_Done(void){
  bool flag = module.done;
  if (flag)
	  module.done = false;
  return flag;
}

/*******************************************************************************
 *                      INTERRUPTS ZONE
 ******************************************************************************/

void FXOS8700CQ_ISR(void){
	if (module.state == FXOS8700CQ_INIT)
		FXOS8700CQ_Config(true);
	else if (module.state == FXOS8700CQ_READY)
		FXOS8700CQ_Run(true);
}

static void FXOS8700CQ_End(void){
  for(uint32_t i = 0 ; i < 1000 ; i++);
  if (module.state == FXOS8700CQ_INIT)
  		FXOS8700CQ_Config(false);
  	else if (module.state == FXOS8700CQ_READY)
  		  FXOS8700CQ_Run(false);
}

static void FXOS8700CQ_Config(bool reset){
	if(reset)
		module.step = 0;
	switch (module.step++){
		case 0:
			// read and check the FXOS8700CQ WHOAMI register
			FXOS8700CQ_Read(FXOS8700CQ_WHOAMI);
			break;

		case 1:
			// write 0000 0000 = 0x00 to accelerometer control register 1 to place FXOS8700CQ into
			// standby
			// [7-1] = 0000 000
			// [0]: active=0
			if (module.I2C_read_queue[1] == FXOS8700CQ_WHOAMI_VAL){
				FXOS8700CQ_Write(FXOS8700CQ_CTRL_REG1, 0x00);		// Standby Mode
			}
			else{
				module.state = FXOS8700CQ_ERROR;		// ERROR
			}
			break;

		case 2:
			// write 0001 1111 = 0x1F to magnetometer control register 1
			// [7]: m_acal=0: auto calibration disabled
			// [6]: m_rst=0: no one-shot magnetic reset
			// [5]: m_ost=0: no one-shot magnetic measurement
			// [4-2]: m_os=111=7: 8x oversampling (for 200Hz) to reduce magnetometer noise
			// [1-0]: m_hms=11=3: select hybrid mode with accel and magnetometer active
			FXOS8700CQ_Write(FXOS8700CQ_M_CTRL_REG1, 0x1F);
			break;

		case 3:
			// write 0010 0000 = 0x20 to magnetometer control register 2
			// [7]: reserved
			// [6]: reserved
			// [5]: hyb_autoinc_mode=1 to map the magnetometer registers to follow the accelerometer registers
			// [4]: m_maxmin_dis=0 to retain default min/max latching even though not used
			// [3]: m_maxmin_dis_ths=0
			// [2]: m_maxmin_rst=0
			// [1-0]: m_rst_cnt=00 to enable magnetic reset each cycle
			FXOS8700CQ_Write(FXOS8700CQ_M_CTRL_REG2, 0x20);
			break;

		case 4:
			// write 0000 0001= 0x01 to XYZ_DATA_CFG register
			// [7]: reserved
			// [6]: reserved
			// [5]: reserved
			// [4]: hpf_out=0
			// [3]: reserved
			// [2]: reserved
			// [1-0]: fs=01 for accelerometer range of +/-4g range with 0.488mg/LSB
			FXOS8700CQ_Write(FXOS8700CQ_XYZ_DATA_CFG, 0x01);
			break;

		case 5:
			// write 0000 1101 = 0x0D to accelerometer control register 1
			// [7-6]: aslp_rate=00
			// [5-3]: dr=001 for 200Hz data rate (when in hybrid mode)
			// [2]: lnoise=1 for low noise mode
			// [1]: f_read=0 for normal 16 bit reads
			// [0]: active=1 to take the part out of standby and enable
			FXOS8700CQ_Write(FXOS8700CQ_CTRL_REG1, 0x0D);
			break;
		case 6:
			module.state = FXOS8700CQ_READY;		// Config READY
			break;

		default:
			break;
	}
}

static void FXOS8700CQ_Run(bool reset){
	if (reset){
		Acceleration_Type a = {0, 0, 0};
		module.step = 0;
		module.accel[(module.index + 1) % 2] = a;
	}

	switch (module.step++){
	case 0:
		FXOS8700CQ_Read(OUT_X_MSB);		// Read Accel X MSB
		break;

	case 1:
		module.accel[(module.index + 1) % 2].x = (module.I2C_read_queue[1] << 8);		// Save Accel X MSB
	  	FXOS8700CQ_Read(OUT_X_LSB);		// Read Accel X LSB
	  	break;

	case 2:
		module.accel[(module.index + 1) % 2].x |= (module.I2C_read_queue[1]);		// Save Accel X LSB
	  	module.accel[(module.index + 1) % 2].x >>= 2;
	  	FXOS8700CQ_Read(OUT_Y_MSB);		// Read Accel Y MSB
	  	break;

	case 3:
		module.accel[(module.index + 1) % 2].y = (module.I2C_read_queue[1] << 8);		// Save Accel Y MSB
		FXOS8700CQ_Read(OUT_Y_LSB);		// Read Accel Y LSB
		break;

	case 4: // Start reading acceleration Z MSB
		module.accel[(module.index + 1) % 2].y |= (module.I2C_read_queue[1]);		// Save Accel Y LSB
		module.accel[(module.index + 1) % 2].y >>= 2;
		FXOS8700CQ_Read(OUT_Z_MSB);		// Read Accel Z MSB
		break;

	case 5: // Start reading acceleration Z LSB
		module.accel[(module.index + 1) % 2].z = (module.I2C_read_queue[1] << 8);		// Save Accel Z MSB
		FXOS8700CQ_Read(OUT_Z_LSB);		// Read Accel Z LSB
		break;
	case 6:
		module.accel[(module.index + 1) % 2].z |= (module.I2C_read_queue[1]);		// Save Accel Z LSB
		module.accel[(module.index + 1) % 2].z >>= 2;
		module.accel_buffer[1] = module.accel_buffer[0];		// Buffer for sending latest value of Acel when I2C Com not done
		module.accel_buffer[0] = module.accel[1];
		module.done = true;		// done flag
		break;
	default:
		break;
	}
}

static void FXOS8700CQ_Read(uint8_t reg){
	module.I2C_write_queue[0] = reg;		// FXOS8700CQ Register to Read
	I2C_Communication_Config(0, FXOS8700CQ_SLAVE_ADDR, module.I2C_write_queue, module.I2C_read_queue, 1, 1);		// Config I2C Com
}

static void FXOS8700CQ_Write(uint8_t reg, uint8_t data){
	module.I2C_write_queue[0] = reg;		// FXOS8700CQ Register to Write
	module.I2C_write_queue[1] = data;		// Data to Write
	I2C_Communication_Config(0, FXOS8700CQ_SLAVE_ADDR, module.I2C_write_queue, NULL, 2, 0);		// Config I2C Com
}

bool FXOS8700CQ_Is_Running(void){
  return (module.state == FXOS8700CQ_READY);
}

Acceleration_Type FXOS8700CQ_get_Acceleration(void){
	Acceleration_Type a = {0,0,0};
	if (module.state == FXOS8700CQ_READY)
		a = module.accel_buffer[0];
	return a;
}

int16_t FXOS8700CQ_get_Accel_x(void){
	int16_t ax = 0;
	if (module.state == FXOS8700CQ_READY)
		ax = module.accel_buffer[0].x;
	return ax;
}

int16_t FXOS8700CQ_get_Accel_y(void){
	int16_t ay = 0;
	if (module.state == FXOS8700CQ_READY)
		ay = module.accel_buffer[0].y;
	return ay;
}

int16_t FXOS8700CQ_get_Accel_z(void){
	int16_t az = 0;
	if (module.state == FXOS8700CQ_READY)
		az = module.accel_buffer[0].z;
	return az;
}
