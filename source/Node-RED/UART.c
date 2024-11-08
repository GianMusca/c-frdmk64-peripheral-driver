/***************************************************************************//**
  @file     UART.c
  @brief    UART functions
  @author   Ale
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "MK64F12.h"
#include "board.h"
#include "hardware.h"
#include "UART.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

#define MAX_BAUD_RATE 			((uint32_t)0xFFFF)	//See Section 52.4.3 of reference manual
#define MIN_BAUD_RATE 			0x0000
#define BAUD_RATE_DEFAULT 		9600				//Used in general purpose applications

#define DISABLE 0
#define UART_MUX_ALT 3			//Used in port mux, see Section 10.3.1 for alternatives table

#define RX_BUFFER_LENGTH 	2000
#define TX_BUFFER_LENGTH 	2000

#define MAX_UART_ID		6		//K64 Supports up to 6 UART coms, see Section 52.3 of reference manual for registers

#define EMPTY 0

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/


/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL LEVEL SCOPE
 ******************************************************************************/

static UART_mode UART_modes[MAX_UART_ID] = { UART_BLOCKING, UART_BLOCKING, UART_BLOCKING, UART_BLOCKING, UART_BLOCKING, UART_BLOCKING};

static const UART_Type * UART_PTRS[] = UART_BASE_PTRS;	//Array initializer, see MK64F12.h line 11785

/********************** RX ****************************/
static uint8_t UART_RX_PORTS[] = { PB, PC, PD, PC, PE, PD};	//Order according to reference manual, from UART0 to UART5
static uint8_t UART_RX_NUM[] =  { 16,  3,  2, 16, 24,  9};	//RX UART modules pins , see Section 10.3.1

static char RX_buffer[MAX_UART_ID][RX_BUFFER_LENGTH]; 		//Buffers for each UART, rows are ID, cols are that ID's buffer
//Used for each UART's latest character received
static int last_RX_count[MAX_UART_ID] = { EMPTY, EMPTY, EMPTY, EMPTY, EMPTY, EMPTY};
//Each flag is used to know when a UART is reading
static bool reading[MAX_UART_ID] = { false, false, false, false, false, false};	//Flag to set if not to be interrupted, used in copy
//static uint8_t counters_RX_failed[MAX_UART_ID] = { 0, 0, 0, 0, 0 };


/********************** TX ****************************/
static uint8_t UART_TX_PORTS[] = { PB, PC, PD, PC, PE, PD};	//Ports for each TX, from UART0 to UART5
static uint8_t UART_TX_NUM[] =  { 17,  4,  3, 17, 25,  8 };//Pins for each TX

static char TX_buffer[MAX_UART_ID][TX_BUFFER_LENGTH];
static uint32_t last_TX_count[MAX_UART_ID] = { 0, 0, 0, 0, 0, 0};	//Contains the amount of bytes received for each UART, clean after every transfer
static uint32_t last_TX_info[MAX_UART_ID] = { 0, 0, 0, 0, 0, 0};	//Contains the index of the next byte to transmit for each UART
static uint32_t last_TX_buffer[MAX_UART_ID] = { 0, 0, 0, 0, 0, 0};	//Contains the amount of bytes left to transmit for each UART

//interruptions
uint8_t UART_RX_TX_IRQ_arr[] = UART_RX_TX_IRQS;
//uint8_t UART_RX_TX_ERR_arr[] = UART_ERR_IRQS;

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

bool is_valid_UART_id(UART_id id);
void UART_set_clock_gating(UART_id id);
void UART_disable_coms(UART_id id);
void UART_set_baud_rate(UART_id id, uint32_t baud_rate);
void UART_set_parity(UART_id id, UART_parity parity);
void UART_set_stop_bit(UART_id id);
void UART_set_ports(UART_id id);
void UART_disable_IRQ(UART_id id);
void UART_set_mode(UART_id id, UART_mode mode);
void UART_start_rx(UART_id id);

void UART_write_blocking(UART_id id, const char* msg, uint32_t length);
bool UART_write_nonblocking(UART_id id, const char* msg, uint32_t length);

void UARTX_RX_TX_IRQHandler(UART_id id);
void UART0_RX_TX_IRQHandler();					//Names taken from hardware.h
void UART1_RX_TX_IRQHandler();
void UART2_RX_TX_IRQHandler();
void UART3_RX_TX_IRQHandler();
void UART4_RX_TX_IRQHandler();
void UART5_RX_TX_IRQHandler();
/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
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
void UART_init (UART_id id, uint32_t baud_rate, UART_mode mode, UART_parity parity, UART_nbits nbits){

	if (is_valid_UART_id(id)){ //Checks valid ID
		//uart_p = UART_POINTERS[id];

		UART_set_clock_gating(id);
		UART_disable_coms(id);
		UART_set_baud_rate(id, baud_rate);
		UART_set_parity(id, parity);

		//uart_p->BDH &= ~UART_BDH_SBNS_MASK;
		UART_set_stop_bit(id);

		UART_set_ports(id);
		UART_disable_IRQ(id);
		UART_set_mode(id, mode);
		UART_start_rx(id);
		UART_modes[id] = mode;
	}
}

/**
 * @brief checks the state of reception of data
 * @param id: ID of the UART to check
 * @return weather or not the UART is receiving data
 */
bool UART_is_new_info (UART_id id){
	UART_Type *UART_ptr = UART_PTRS[id];
	bool new_info = false;

	if(is_valid_UART_id(id)){
		if(UART_modes[id] == UART_BLOCKING){
			new_info = UART_ptr->S1 & UART_S1_RDRF_MASK;	//See Section 52.3.5 for UART Status Register 1, RDRF: Receive Data Register Full Flag
		}
		else if(UART_modes[id] == UART_NON_BLOCKING){
			if(last_RX_count[id] != EMPTY){	//If there is new data, then return true
				new_info = true;
			}
		}
	}
	return new_info;
}

/**
 * @brief Used to know how many bytes of data are stored in the RX buffer
 * @param id: ID of the UART to check
 * @return amount of bytes stored in the RX buffer
 */
uint32_t UART_get_msg_length (UART_id id){
	uint32_t length = last_RX_count[id] + 1;			//The length + 1 counts next byte
	if(!is_valid_UART_id(id))
		length = 0;
	return length;
}

/**
 * @brief Transfer data from RX buffer to an external buffer, recomended to use UART_get_msg_length(id) before this
 * @param id: ID of the UART to check
 * @param buff: buffer to transfer the received data
 * @param length: amount of bytes to transfer
 * @return
 */
void UART_read (UART_id id, char* buff, uint32_t length){
	UART_Type *UART_ptr = UART_PTRS[id];
	uint32_t i = 0;
	uint32_t j = 0;
	if(is_valid_UART_id(id)){
		if(UART_modes[id] == UART_BLOCKING){
			while( (UART_ptr->S1 & UART_S1_RDRF_MASK) && (i<length)){		//While there are new bytes, and haven't finished copying
				i++;		//Next byte
				buff[i] = UART_ptr->D;		//See Section 52.3.8 for UART Data Register, D: Data, 8 bits
			}
		}
		else if(UART_modes[id] == UART_NON_BLOCKING){
			if(last_RX_count != EMPTY){	//Only copy if there is new info
				reading[id] = true;		//Not to be interrupted while copying data

				for(i=0; i<length; i++)
					buff[i] = RX_buffer[id][i];		//Copy RX buffer to target buffer

				//After copying, data is irrelevant, so we move the uncopied data
				for(j=0; j < last_RX_count[id] + 1- length; j++)	//length points to first byte now, we move them to the beginning
					RX_buffer[id][j] = RX_buffer[id][length+j];

				if(j == 0)
					last_RX_count[id] = EMPTY;	//If copied all bytes, no more info is available
				else
					last_RX_count[id] = j-1;		//Otherwise, j points to the FIRST EMPTY byte, so we decrease

				reading[id] = false;	//Done copying, interruptions allowed
			}
		}
	}
	return;
}

/**
 * @brief checks the state of transmission of data
 * @param id: ID of the UART to check
 * @return weather or not the UART is sending data
 */
bool UART_is_TX_done (UART_id id){
	bool done = false;
	UART_Type *UART_ptr = UART_PTRS[id];
	if(is_valid_UART_id(id)){
		done = UART_ptr->C2 & UART_C2_TCIE_MASK;	//See Section 52.3.4 for UART Control Register 2, TCIE: Transmission Complete Interrupt or DMA Transfer Enable
	}
	return done;
}

/**
 * @brief Send data through the chosen UART
 * @param id: ID of the UART to use
 * @param msg: buffer where the message is stored
 * @param length: amount of bytes to transfer (transmission starts at msg[0])
 * @return if the msg was successfully stored to be transmitted
 */
bool UART_write(UART_id id, const char* msg, uint32_t length){
	bool stored = true;
	if(is_valid_UART_id(id)){
		if(UART_modes[id] == UART_BLOCKING)
			UART_write_blocking(id, msg, length);
		else if(UART_modes[id] == UART_NON_BLOCKING)
			stored = UART_write_nonblocking(id, msg, length);
	}
	return stored;
}

/*******************************************************************************
 *******************************************************************************
                        PRIVATE FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

/**
 * @brief checks if an UART_id is valid
 * @param id: ID of the UART to check
 * @return weather or not the UART_id is valid
 */
bool is_valid_UART_id(UART_id id){
	bool valid = false;
	if((id >= 0) && (id < MAX_UART_ID))
		valid = true;
	return valid;
}

/**
 * @brief Enables clock gating for a VALID UART_id
 * @param id: ID of the UART
 * @return
 */
void UART_set_clock_gating(UART_id id){
	SIM_Type *sim = SIM;						//For UART we use System Clock Gating Control Register 1 and 4, see MK64F12.h
	switch(id){									//Enable clock gating for the desired UART
		case UART_0:	sim->SCGC4 |= SIM_SCGC4_UART0_MASK;	break;
		case UART_1:	sim->SCGC4 |= SIM_SCGC4_UART1_MASK;	break;
		case UART_2:	sim->SCGC4 |= SIM_SCGC4_UART2_MASK;	break;
		case UART_3:	sim->SCGC4 |= SIM_SCGC4_UART3_MASK;	break;
		case UART_4:	sim->SCGC4 |= SIM_SCGC1_UART4_MASK;	break;
		case UART_5:	sim->SCGC4 |= SIM_SCGC1_UART5_MASK;	break;
	}
}

/**
 * @brief Disables communications for a VALID UART_id
 * @param id: ID of the UART
 * @return
 */
void UART_disable_coms(UART_id id){
	//See Section 52.3.4 for UART Control Register 2
	//TE = 1 enables TX, and RE = 1 enables RX, so we negate the masks to disable
	UART_Type *UART_ptr = UART_PTRS[id];
	UART_ptr->C2 &= (~(UART_C2_TE_MASK | UART_C2_RE_MASK));
}

/**
 * @brief Sets the desired baud rate for a VALID UART_ID
 * @param id: ID of the UART
 * @param baud_rate: desired baud rate
 * @return
 */
void UART_set_baud_rate(UART_id id, uint32_t baud_rate){
	//From Section 52.3.1: "To update the baud rate, fisrt write to BDH with the high half, then write to BDL"
	//From Section 52.4.3: UART baud rate = UART module clock/ (16*SBR[12:0] + BRFD), BRFD = BRFA/32
	UART_Type *UART_ptr = UART_PTRS[id];
	uint32_t clock;
	uint16_t SBR;
	uint8_t BRFA;

	if(baud_rate <= MIN_BAUD_RATE || baud_rate >= MAX_BAUD_RATE)
		baud_rate = BAUD_RATE_DEFAULT;

	if(id == UART_0 || id == UART_1)
		clock = (__CORE_CLOCK__);		//Warning! UART0 and UART1 use CORE_CLOCK, whereas the rest use BUS_CLOCK
	else
		clock = (__CORE_CLOCK__ >> 1);

	SBR = clock/ (baud_rate << 4);					//Without fine adjust, SBR = UART module clock / (baud_rate/16)
	BRFA = ((clock << 1) / baud_rate) - (SBR << 5);	//BRFA = (2*module_clock/baud_rate) - 32*SBR

	//Set registers BDH, then BDL, then inform BRFA to UART Control Register 4, see Section 52.3.11
	UART_ptr->BDH = UART_BDH_SBR((SBR >> 8));		//Only 5 MSB of SBR go into BDH, AND SBR is SBR[12:0], but used as uint16_t here
	UART_ptr->BDL = UART_BDL_SBR(SBR);				//The 8 LSB go into BDL
	UART_ptr->C4 = (UART_ptr->C4 & (~UART_C4_BRFA_MASK)) | UART_C4_BRFA(BRFA);	//See Section 52.3.11
}

/**
 * @brief Sets the desired parity type for a VALID UART_id
 * @param id: ID of the UART
 * @param parity: parity for the desired UART communication
 * @return
 */
void UART_set_parity(UART_id id, UART_parity parity){
	//See Section 52.3.3 for UART Control Register 1 for PE: Parity Enable, PT: Partity Type, and M: Mode
	UART_Type *UART_ptr = UART_PTRS[id];
	switch(parity){
		case UART_NO_PARITY:
			UART_ptr->C1 &= ~UART_C1_M_MASK;
			UART_ptr->C1 &= ~UART_C1_PE_MASK;	//For no parity, just disable parity
			break;
		case UART_EVEN:
			UART_ptr->C1 |= UART_C1_PE_MASK;
			UART_ptr->C1 &= ~UART_C1_PT_MASK;
			UART_ptr->C1 |= UART_C1_M_MASK;
		break;
		case UART_ODD:
			UART_ptr->C1 |= UART_C1_PE_MASK;
			UART_ptr->C1 |= UART_C1_PT_MASK;
			UART_ptr->C1 |= UART_C1_M_MASK;
			break;
	}
}

/**
 * @brief Sets the number of stop bits for a VALID UART_id
 * @param id: ID of the UART
 * @return
 */
void UART_set_stop_bit(UART_id id){
	//See Section 52.3.1 for place of SBNS: Stop Bit Number Select, we set to 0, so just a single stop bit
	UART_Type *UART_ptr = UART_PTRS[id];
	UART_ptr->BDH &= ~UART_BDH_SBNS_MASK;
}

/**
 * @brief Sets the ports for RX and TX for a VALID UART_id
 * @param id: ID of the UART
 * @return
 */
void UART_set_ports(UART_id id){
	PORT_Type *PORT_PTRS[] = PORT_BASE_PTRS;

	(PORT_PTRS[UART_RX_PORTS[id]]->PCR)[UART_RX_NUM[id]] &= ~PORT_PCR_MUX_MASK;				//Enable Port for RX
	(PORT_PTRS[UART_RX_PORTS[id]]->PCR)[UART_RX_NUM[id]] |= PORT_PCR_MUX(UART_MUX_ALT);		//Set alternative for UART

	(PORT_PTRS[UART_TX_PORTS[id]]->PCR)[UART_TX_NUM[id]] &= ~PORT_PCR_MUX_MASK;				//Enable Port for TX
	(PORT_PTRS[UART_TX_PORTS[id]]->PCR)[UART_TX_NUM[id]] |= PORT_PCR_MUX(UART_MUX_ALT);		//Set alternative for UART

}

/**
 * @brief Disables interrupts for a VALID UART_id
 * @param id: ID of the UART
 * @return
 */
void UART_disable_IRQ(UART_id id){
	PORT_Type *PORT_PTRS[] = PORT_BASE_PTRS;

	(PORT_PTRS[UART_RX_PORTS[id]]->PCR)[UART_RX_NUM[id]] &= ~PORT_PCR_IRQC_MASK;
	(PORT_PTRS[UART_RX_PORTS[id]]->PCR)[UART_RX_NUM[id]] |= PORT_PCR_IRQC(DISABLE);	//Disable interrupts

	(PORT_PTRS[UART_TX_PORTS[id]]->PCR)[UART_TX_NUM[id]] &= ~PORT_PCR_IRQC_MASK;
	(PORT_PTRS[UART_TX_PORTS[id]]->PCR)[UART_TX_NUM[id]] |= PORT_PCR_IRQC(DISABLE);	//Disable interrupts
}

/**
 * @brief Sets configuration for either BLOCKING or NON_BLOCKING for a VALID UART_id
 * @param id: ID of the UART
 * @return
 */
void UART_set_mode(UART_id id, UART_mode mode){
	UART_Type *UART_ptr = UART_PTRS[id];
	if(mode == UART_NON_BLOCKING){
		NVIC_EnableIRQ(UART_RX_TX_IRQ_arr[id]);			//Enable interrupts for that UART
		UART_ptr->C2 |= UART_C2_RIE_MASK;			//See Section 52.3.4 for UART Control Register 2
	}
}

/**
 * @brief Starts communications for a VALID UART_id
 * @param id: ID of the UART
 * @return
 */
void UART_start_rx(UART_id id){
	UART_Type *UART_ptr = UART_PTRS[id];
	UART_ptr->C2 |= UART_C2_RE_MASK;	//See Section 52.3.4 for UART Control Register 2
}

/**
 * @brief Sends a msg through UART, blocking, assumes correct UART_id
 * @param id: ID of the UART
 * @param msg: pointer to message to be transmitted
 * @param length: amount of bytes to transmit, starts at msg[0]
 * @return
 */
void UART_write_blocking(UART_id id, const char* msg, uint32_t length){
	UART_Type *UART_ptr = UART_PTRS[id];
	uint32_t i = 0;
	for(i = 0; i<length; i++)
		UART_ptr->D = msg[i];
}

/**
 * @brief Sends a msg through UART, nonblocking, assumes correct UART_id
 * @param id: ID of the UART
 * @param msg: pointer to message to be transmitted
 * @param length: amount of bytes to transmit, starts at msg[0]
 * @return if the msg was successfully stored to be transmitted
 */
bool UART_write_nonblocking(UART_id id, const char* msg, uint32_t length){
	/*bool stored = false;
	uint32_t i;

	if((last_TX_buff[id] + length) <= TX_BUFFER_LENGTH){		//If there is space to transmit all data
		lst_TX_buff[id] += length;
		for(i = 0; i<length; i++){
			TX_buffers[id][last_TX_info] = msg[i];
			if(last_TX_info[id] < TX_BUFFER_LENGTH - 1)
				last_TX_info[id] += 1;		//If there is still space, advance
			else
				last_TX_info[id] = 0;		//Otherwise, finish

			last_TX_buff[id] += 1;			//Increment amount of bytes in the buffer
		}
		if(~(UART_ptr->C2 & UART_C2_TCIE_MASK)){	//If finished transmitting or UART wasn't used
			UART_ptr->C2 |= UART_C2_TE_MASK;		//Set interrupt flags
			UART_ptr->C2 |= UART_C2_TIE_MASK;
		}
		stored = true;
	}
	return stored;*/
	UART_Type *UART_ptr = UART_PTRS[id];
	bool stored = false;
	bool working = false;
	uint32_t i = 0;

	if(last_TX_buffer[id] == 0){
		working = true;
	}
	if(length < TX_BUFFER_LENGTH - last_TX_buffer[id]){
		for(i=0; i<length; i++){
			TX_buffer[id][last_TX_info[id]] = msg[i];
			if(last_TX_info[id] < TX_BUFFER_LENGTH - 1)
				last_TX_info[id]++;
			else
				last_TX_info[id] = 0;
			last_TX_buffer[id]++;
		}
		if(working || ~(UART_ptr->C2 & UART_C2_TCIE_MASK)){
			UART_ptr->C2 |= UART_C2_TE_MASK;
			UART_ptr->C2 |= UART_C2_TIE_MASK;
		}
		stored = true;
	}
	return stored;
}


/**
 * @brief Generic interrupt function for UART receiving and transmitting
 * @param id: ID of the UART
 * @return
 */
void UARTX_RX_TX_IRQHandler(UART_id id){
	//For each UART interrupt, if we have need to receive data, we prioritize that, otherwise we see if there is data to transmit
	UART_Type *UART_ptr = UART_PTRS[id];
	if(UART_ptr->S1 & UART_S1_RDRF_MASK){			//If Receive Data Register Full Flag is 1, there is new data in the RX, so we handle it (see Section 52.3.5)
		if(!reading[id]){							//If that UART is not reading data
			if(last_RX_count[id] < RX_BUFFER_LENGTH - 1){
				RX_buffer[id][last_RX_count[id]] = UART_ptr->D;	//Read the data from Data register
				last_RX_count[id]++;				//Point to next slot in the RX buffer
			}
		}
	}
	else if(UART_ptr->S1 & UART_S1_TDRE_MASK){		//If Transmit Data Register Empty Flag is 1, then we can transmit a byte (See Section 52.3.5)
		if(last_TX_buffer[id] > 0){								//If there is data to transmit
			UART_ptr->D = TX_buffer[id][last_TX_count[id]];	//Transfer next byte to Data register
			if(last_TX_count[id] < TX_BUFFER_LENGTH - 1)		//If circular buffer isn't at the end
				last_TX_count[id]++;
			else												//If circular buffer overflows
				last_TX_count[id] = 0;							//Reset the index
			last_TX_buffer[id]--;								//Now there is one less byte to transmit
		}
	}
}
void UART0_RX_TX_IRQHandler(){
	UARTX_RX_TX_IRQHandler(UART_0);
}
void UART1_RX_TX_IRQHandler(){
	UARTX_RX_TX_IRQHandler(UART_1);
}
void UART2_RX_TX_IRQHandler(){
	UARTX_RX_TX_IRQHandler(UART_2);
}
void UART3_RX_TX_IRQHandler(){
	UARTX_RX_TX_IRQHandler(UART_3);
}
void UART4_RX_TX_IRQHandler(){
	UARTX_RX_TX_IRQHandler(UART_4);
}
void UART5_RX_TX_IRQHandler(){
	UARTX_RX_TX_IRQHandler(UART_5);
}
