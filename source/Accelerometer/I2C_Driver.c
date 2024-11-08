/***************************************************************************//**
  @file     I2C_Driver.c
  @brief    Application functions
  @author   Matías Tripoli
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "I2C_Driver.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define MUL 0x1		// x2	(Kinetis K64 - Reference Manual - pag. 1538)
#define ICR 0x2B	// x512	((Kinetis K64 - Reference Manual - pag. 1553-1554)
//#define I2C_PORT_CONFIG		(PORT_PCR_PS(1) | PORT_PCR_PE(1) | PORT_PCR_MUX(5) | PORT_PCR_IRQC(0) | PORT_PCR_ISF_MASK) // pull-up, Alternative 5, Interrupt/DMA request disabled.
#define I2C_PORT_CONFIG		(PORT_PCR_ODE(1) | PORT_PCR_MUX(5) | PORT_PCR_IRQC(0) | PORT_PCR_ISF_MASK)

#define CONCAT_MACRO(a,b)	CONCAT_MACRO2(a,b)
#define CONCAT_MACRO2(a,b)	a##b

#define I2C_BASE			CONCAT_MACRO(I2C, I2C_MODULE_NUM)	// Definition what register to use

#define I2C_MODULE_NUM		0
#define I2C_PIN_PORT		PORTE
#define I2C_PIN_SDA_NUM		25
#define I2C_PIN_SCL_NUM		24

/*******************************************************************************
 * CONSTANTS AND GLOBAL VARIABLES
 ******************************************************************************/

//static I2C_Type* I2C0;
static I2C_Communication_Type i2c_com;

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

void I2C_ISR_FSM(void);
__ISR__ I2C0_IRQHandler(void);

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void I2C_Init(void){
	static bool inited = false;
	if(!inited){
		// Clock Gating
		SIM->SCGC4 |= SIM_SCGC4_I2C0_MASK;		// I2C0
		SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;		// Port E

		// Set pin as I2C
		I2C_PIN_PORT->PCR[I2C_PIN_SDA_NUM] = I2C_PORT_CONFIG;		// SDA
		I2C_PIN_PORT->PCR[I2C_PIN_SCL_NUM] = I2C_PORT_CONFIG;		// SCL

		// I2C Enable & Interrupt Enable (Kinetis K64 - Reference Manual - 51.3.3 I2C Control Register 1)
		I2C_BASE->C1 = I2C_C1_IICEN_MASK; // I2C Enable
		I2C0->F = I2C_F_ICR(ICR) | I2C_F_MULT(MUL);		// I2C BaudRate Config
		I2C_BASE->S = I2C_S_IICIF_MASK;
		//I2C_BASE->A1 = 0x1A; // dummy address

		NVIC_EnableIRQ(I2C0_IRQn);		//IRQ Enable for I2C0
		//I2C_BASE->C1 = I2C_C1_IICIE_MASK; // I2C Interrupt Enable

		inited = true;
	}
/*		Dany's Example
	// Enable Clock Gating
	SIM->SCGC4 |= SIM_SCGC4_I2C0_MASK;		// I2C0
	SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;		// Port E

	// Enable Pins
	I2C_PIN_PORT->PCR[I2C_PIN_SDA_NUM] = I2C_PORT_CONFIG;		// SDA
	I2C_PIN_PORT->PCR[I2C_PIN_SCL_NUM] = I2C_PORT_CONFIG;		// SCL



	// Configuro periférico I2C

	I2C_BASE->C1 = I2C_C1_IICEN_MASK; // I2C Enable

	I2C_BASE->F = I2C_F_ICR(0x13) | I2C_F_MULT(0x2); // Configuro baudrate I2C

	I2C_BASE->S = I2C_S_IICIF_MASK;

	I2C_BASE->A1 = 0x1A; // dummy address



	// Habilito interrupciones

	NVIC_EnableIRQ(I2C0_IRQn);

	//I2C_BASE->C1 = I2C_C1_IICIE_MASK; // I2C Interrupt Enable
*/
}

void I2C_Communication_Config(bool rnotw, uint32_t s_addr, uint8_t* w_q, uint8_t* r_q, uint8_t w_q_s, uint8_t r_q_s){
	//Init I2C_Communication_Type

	i2c_com.state = I2C_COM_CONFIG;

	i2c_com.readnotwrite = rnotw;
	i2c_com.slave_address = s_addr;
	i2c_com.write_queue = w_q;
	i2c_com.read_queue = r_q;
	i2c_com.write_queue_size = w_q_s;
	i2c_com.read_queue_size = r_q_s;
	i2c_com.writing_byte_counter = 0;
	i2c_com.reading_byte_counter = 0;

	// Clear all flags before begin

	I2C0->FLT = I2C_FLT_STARTF_MASK | I2C_FLT_STOPF_MASK ;		// Clear FLT flags (START and STOP); START-STOP ISR disable

	I2C0->S = I2C_S_IICIF_MASK | I2C_S_ARBL_MASK;		// Clear IICIF flag

	// START + ADDRESS
	I2C0->C1 |= I2C_C1_TX_MASK | I2C_C1_MST_MASK | I2C_C1_IICIE_MASK;		// set TX mode and generate START and enable ISR

	I2C0->D = (uint8_t) (s_addr<<1) | (0<<0);		// TX address + RnW = 0 ** SE DEBE HACER INMEDIATAMENTE DEPUÉS DE CONFIGURAR C1 ****
}

I2C_State_Type I2C_get_State(void){
	I2C_State_Type state = i2c_com.state;
	if (state == I2C_END || state == I2C_ERROR)
		i2c_com.state = I2C_IDLE;
	return state;
}

void I2C_End_Routine(I2C_Callback_Type callback){
	i2c_com.end_routine = callback;
}
/*******************************************************************************
                        INTERRUPTS ZONE
 ******************************************************************************/

void I2C_ISR_FSM(void){		// FSM-ish
	// save register S
	uint8_t status_r = I2C0->S;
	I2C0->S = I2C_S_TCF_MASK | I2C_S_IICIF_MASK;		// w1c Interrupt Flag

	if(status_r & I2C_S_TCF_MASK){		// Transfer Complete?
		if(I2C0->C1 & I2C_C1_TX_MASK){		// Tx?
			if(i2c_com.writing_byte_counter == i2c_com.write_queue_size){		// No byte left to Tx?
				if(i2c_com.read_queue_size){		// Anything to read?
					if(I2C0->S & I2C_S_RXAK_MASK){		// NACK form Slave?
						i2c_com.state = I2C_ERROR;
						I2C0->C1 = (I2C0->C1 & ~I2C_C1_MST_MASK) | I2C_C1_MST(0);
					}
					else{		// ACK from Slave?
						if(i2c_com.readnotwrite){		// Reading?
							I2C0->C1 = (I2C0->C1 & ~I2C_C1_TX_MASK) | I2C_C1_TX(0);		// Stop Tx
							if(i2c_com.read_queue_size == 1){		// Only 1?
								I2C0->C1 = (I2C0->C1 & ~I2C_C1_TXAK_MASK) | I2C_C1_TXAK(1);		// Send NACK
							}
							I2C0->D;		// Read D to advance
						}
						else{		// not Reading yet?
							I2C0->C1 = (I2C0->C1 & ~I2C_C1_RSTA_MASK) | I2C_C1_RSTA(1);		// Transmit Repeated Start
							I2C0->D = ((i2c_com.slave_address & 0x0000007F) << 1) | 0x00000001;		// Transmit Slave Address + Read Condition
							i2c_com.readnotwrite = true;		// Stop writing
						}
					}
				}
				else{		// Nothing to Read
					i2c_com.state = I2C_END;
					I2C0->C1 = (I2C0->C1 & ~I2C_C1_MST_MASK) | I2C_C1_MST(0);
					i2c_com.end_routine();
				}
			}
			else if(i2c_com.writing_byte_counter < i2c_com.write_queue_size){
				if(I2C0->S & I2C_S_RXAK_MASK){		// NACK form Slave?
					i2c_com.state = I2C_ERROR;
					I2C0->C1 = (I2C0->C1 & ~I2C_C1_MST_MASK) | I2C_C1_MST(0);
				}
				else{
					I2C0->D = i2c_com.write_queue[i2c_com.writing_byte_counter++];		// Send next byte
				}
			}
		}
		else{		// Rx
			if(i2c_com.reading_byte_counter == i2c_com.read_queue_size - 1){		// Last byte left to Rx?
				I2C0->C1 = (I2C0->C1 & ~I2C_C1_MST_MASK) | I2C_C1_MST(0);
				i2c_com.read_queue[i2c_com.read_queue_size++] = I2C0->D;		// Receive next byte
				i2c_com.state = I2C_END;
				i2c_com.end_routine();
			}
			else{		// bytes left to Read
				if(i2c_com.reading_byte_counter == i2c_com.read_queue_size - 2)
					I2C0->C1 = (I2C0->C1 & ~I2C_C1_TXAK_MASK) | I2C_C1_TXAK(1);		// Send NACK
				i2c_com.read_queue[i2c_com.reading_byte_counter++] = I2C0->D;		// Receive next byte
			}
		}
	}
}

__ISR__ I2C0_IRQHandler(void){
	I2C_ISR_FSM();
}
