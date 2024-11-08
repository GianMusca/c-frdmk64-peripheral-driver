/***************************************************************************//**
  @file     App.c
  @brief    Application functions
  @author   Ale
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "MK64F12.h"
#include "hardware.h"
#include "board.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define CG_ON	1
#define CG_OFF	0

#define DFWR_MAX	32
/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/
//Masks used in clock gating operations
const static uint32_t SIM_SCGC5_PORT_MASKS[] = {SIM_SCGC5_PORTA_MASK, SIM_SCGC5_PORTB_MASK, SIM_SCGC5_PORTC_MASK, SIM_SCGC5_PORTD_MASK, SIM_SCGC5_PORTE_MASK};
//Port arrays used in port selection
static PORT_Type*const PORT_SELECT[] = {PORTA, PORTB, PORTC, PORTD, PORTE};
//GPIO array used in GPIO selection
static GPIO_Type*const GPIO_SELECT[] = {PTA, PTB, PTC, PTD, PTE};

//Matriz de callbacks de cada puerto, cada pin
pinIRQFunction_t port_pin_interrupt[5][32];
uint32_t pin_irq_configured[5][32];

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/
static void gpioCG (pin_t pin, uint8_t mode);
static void interrupt_init(uint32_t IRQn);

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
/**
 * @brief Configures clock gating mode for a preipheral
 * @param pin: the pin whose clock gating mode you wish to set (according PORTNUM2PIN)
 * @param mode GC_ON, GC_OFF.
 */
static void gpioCG (pin_t pin, uint8_t mode){
	if(mode == CG_ON){										//If turning on clock gating
		SIM->SCGC5 |= SIM_SCGC5_PORT_MASKS[PIN2PORT(pin)];	//OR with pin mask
	}
	else if(mode == CG_OFF){								//If turning off clock gating
		SIM->SCGC5 &= ~SIM_SCGC5_PORT_MASKS[PIN2PORT(pin)]; //AND with NEG pin mask
	}
}


/**
 * @brief Configures the specified pin to behave either as an input or an output
 * @param pin the pin whose mode you wish to set (according PORTNUM2PIN)
 * @param mode 		INPUT, OUTPUT, INPUT_PULLUP or INPUT_PULLDOWN.
 * @param DSE_mode		Drive Strength Enable: 		DSE_DEFAULT, DSE_HIGH_STRENGTH, DSE_LOW_STRENGTH
 * @param ODE_mode		Open Drain Enable: 			ODE_DEFAULT, ODE_ENABLE, ODE_DISABLE
 * @param SRE_mode		Slew Rate Enable: 			SRE_DEFAULT, SRE_SLOW, SRE_FAST
 * @param PFE_mode		Passive Filter Enable: 		PFE_DEFAULT, PFE_ENABLE, PFE_DISABLE
 * @param DFE_mode		Digital Filter Enable: 		DFE_DEFAULT, DFE_ENABLE, DFE_DISABLE
 * @param DFW_width		Digital Filter Width		5bit value for how many clock cycles are ignored for input changes, only supported
 * on pins that DO NOT  support a digital filter. Use 0 for no filter.
 */
void gpioMode (pin_t pin, uint8_t mode, uint8_t DSE_mode, uint8_t ODE_mode, uint8_t SRE_mode, uint8_t PFE_mode, uint8_t DFE_mode, uint8_t DFW_width){

	//Enable Port Clock Gating
	gpioCG(pin, CG_ON);

	//Pin Control Register (PCR) CONFIG
	uint8_t port = PIN2PORT(pin);
	uint32_t number = PIN2NUM(pin);
	PORT_Type*  port_pointer = PORT_SELECT[port];
	GPIO_Type* gpio_pointer = GPIO_SELECT[port];

	port_pointer->PCR[number] = 0x00;
	// Set pin as GPIO
	port_pointer->PCR[number] |= PORT_PCR_MUX(PORT_mGPIO);
	port_pointer->PCR[number] |= PORT_PCR_IRQC(0);

	if (mode == OUTPUT){							//If set to output
		gpio_pointer->PDDR |= (1<<number);			//Configure PDDR register of GPIO to its pin number
		//DSE
		if(DSE_mode == DSE_HIGH_STRENGTH)
			port_pointer->PCR[number] |= PORT_PCR_DSE(number); 		// HIGH DRIVE STRENGTH
		else if (DSE_mode == DSE_LOW_STRENGTH)
			port_pointer->PCR[number] &= ~PORT_PCR_DSE(number); 	// LOW DRIVE STRENGTH
		//ODE
		if(ODE_mode == ODE_ENABLE)
			port_pointer->PCR[number] |= PORT_PCR_ODE(number); 		// OPEN DRAIN ENABLE
		else if(ODE_mode == ODE_DISABLE)
			port_pointer->PCR[number] &= ~PORT_PCR_ODE(number); 	// OPEN DRAIN DISABLE
		//SRE
		if(SRE_mode == SRE_SLOW)
			port_pointer->PCR[number] |= PORT_PCR_SRE(number); 		// SLOW SLEW RATE
		else if(SRE_mode == SRE_FAST)
			port_pointer->PCR[number] &= ~PORT_PCR_SRE(number); 	// FAST SLEW RATE


	}
	else{
		// Set pin as INPUT
		gpio_pointer->PDDR &= ~(1<<number);			//Configure PDDR register of GPIO to its negated pin number

		switch (mode) {								//Setting PULLUP/DOWN, see Kamman page 10
			case INPUT_PULLUP:
				port_pointer->PCR[number] |= PORT_PCR_PE(number); 	// PULL ENABLE
				port_pointer->PCR[number] |= PORT_PCR_PS(1); 		//SET PULL UP
				break;
			case INPUT_PULLDOWN:
				port_pointer->PCR[number] |= PORT_PCR_PE(number); 	// PULL ENABLE
				port_pointer->PCR[number] |= PORT_PCR_PS(0);		//SET PULL DOWN
				break;
			default:
				break;
		}
		if(DFW_width != 0){												//DFWR config must be done before all other digital filtering config
			if(DFW_width > 0 && DFW_width <= DFWR_MAX)					//See K64 Reference Manual - page 294.
				port_pointer->DFWR |= PORT_DFWR_FILT(DFW_width);
			else if(DFW_width > DFWR_MAX)
				port_pointer->DFWR |= PORT_DFWR_FILT(DFWR_MAX);
		}
		//PFE
		if(PFE_mode == PFE_ENABLE)
			port_pointer->PCR[number] |= PORT_PCR_PFE(number); 		// PASSIVE FILTER ENABLE
		else if(PFE_mode == PFE_DISABLE)
			port_pointer->PCR[number] &= ~PORT_PCR_PFE(number); 	// PASSIVE FILTER DISABLE
		//DFE
		if(DFE_mode == DFE_ENABLE)
			port_pointer->DFER |= PORT_DFER_DFE(number); 	// DIGITAL FILTER ENABLE
		else if(DFE_mode == DFE_DISABLE)
			port_pointer->DFER &= ~PORT_DFER_DFE(number); 	// DIGITAL FILTER DISABLE
	}
}

/**
 * @brief Write a HIGH or a LOW value to a digital pin
 * @param pin the pin to write (according PORTNUM2PIN)
 * @param val Desired value (HIGH or LOW)
 */
void gpioWrite (pin_t pin, bool value){
	uint8_t port = PIN2PORT(pin);
	uint32_t number = PIN2NUM(pin);
	GPIO_Type* gpio_pointer = GPIO_SELECT[port];

	if (value == HIGH){
		gpio_pointer->PDOR |= (1<<number);				//Use PDOR: Port Data Output Register, see Kammann
	}
	else if(value == LOW){
		gpio_pointer->PDOR &= ~(1<<number);
	}
}

/**
 * @brief Toggle the value of a digital pin (HIGH<->LOW)
 * @param pin the pin to toggle (according PORTNUM2PIN)
 */
void gpioToggle (pin_t pin){
	uint32_t port = PIN2PORT(pin);
	uint32_t number = PIN2NUM(pin);
	GPIO_Type* gpio_pointer = GPIO_SELECT[port];
	gpio_pointer->PTOR |= (1<<number);					//Use PTOR: Port Toggle Output Register, see Kammann page 13
}

/**
 * @brief Reads the value from a specified digital pin, either HIGH or LOW.
 * @param pin the pin to read (according PORTNUM2PIN)
 * @return HIGH or LOW
 */
bool gpioRead (pin_t pin){
	uint32_t port = PIN2PORT(pin);
	uint32_t number = PIN2NUM(pin);
	GPIO_Type* gpio_pointer = GPIO_SELECT[port];
	bool read =  gpio_pointer->PDIR & (1<<number);		//Use PDIR: Port Data Input Register, see Kammann page 13
	return read;
}

/*******************************************************************************
                        INTERRUPTS ZONE
 ******************************************************************************/
/**
 * @brief Configures pin to IRQ events
 * @param pin: pin to set IRQ, use NUM2PIN
 * @param IRQMode: see IRQC modes enum definition at top of gpio.h
 * @param IRQFunction: function to call at pin interrupt triggered
 * @return validation of config
 */
bool gpioIRQ (pin_t pin, uint8_t IRQMode, pinIRQFunction_t IRQFunction){

	uint32_t port = PIN2PORT(pin);      					//Save PORT
	uint32_t number = PIN2NUM(pin);							//Save PORT in NUM form
	PORT_Type*  port_pointer = PORT_SELECT[port];			//Point to PORT
	uint32_t IRQn = PORTA_IRQn + port;						//offset by PORTA_IRQn and port

	bool ret = true;										//Variable for valid execution

	if(IRQMode == GPIO_IRQ_MODE_DISABLE){					//Secure 0 at IRQ if disabled
		pin_irq_configured[port][number] = 0;
	}
	else{													//Otherwise, enable IRQ
		pin_irq_configured[port][number] = 1;
	}
	port_pin_interrupt[port][number] = IRQFunction;  		//Save callback assigned to desired pin

	port_pointer->PCR[number] &= ~PORT_PCR_IRQC_MASK;		//Mask for IRQC
	port_pointer->PCR[number] |= PORT_PCR_IRQC(IRQMode); 	//Set mode for IRQ

	interrupt_init(IRQn);									//Initialize interrupt configuration for pin
	return ret;
}

static void interrupt_init(uint32_t IRQn){

	if ( !(NVIC_GetEnableIRQ(IRQn)) ){  					//Enable interrupts for desired port
			NVIC_EnableIRQ(IRQn);
		}
}

__ISR__ PORTA_IRQHandler(void){
	uint32_t i = 0;
	uint32_t exit_cond = 0;
	while(!exit_cond){
		if(pin_irq_configured[PA][i] == 1){
			if((PORTA->PCR[i] & PORT_PCR_ISF_MASK) == PORT_PCR_ISF_MASK){
				PORTA->PCR[i] |= PORT_PCR_ISF_MASK;
				port_pin_interrupt[PA][i]();
				exit_cond = 1;
			}
		}
		i++;
		if(i == 32){
			exit_cond = 1;
		}
	}
}
__ISR__ PORTB_IRQHandler(void){
	uint32_t i = 0;
	uint32_t exit_cond = 0;
	while(!exit_cond){
		if(pin_irq_configured[PB][i] == 1){

			if((PORTB->PCR[i] & PORT_PCR_ISF_MASK) == PORT_PCR_ISF_MASK){
				PORTB->PCR[i] |= PORT_PCR_ISF_MASK;
				port_pin_interrupt[PB][i]();
				exit_cond = 1;
			}
		}
		i++;
		if(i == 32){
			exit_cond = 1;
		}
	}
}
__ISR__ PORTC_IRQHandler(void){
	uint32_t i = 0;
	uint32_t exit_cond = 0;
	while(!exit_cond){
		if(pin_irq_configured[PC][i] == 1){
			if((PORTC->PCR[i] & PORT_PCR_ISF_MASK) == PORT_PCR_ISF_MASK){
				PORTC->PCR[i] |= PORT_PCR_ISF_MASK;
				port_pin_interrupt[PC][i]();
				exit_cond = 1;
			}
		}
		i++;
		if(i == 32){
			exit_cond = 1;
		}
	}
}
__ISR__ PORTD_IRQHandler(void){
	uint32_t i = 0;
	uint32_t exit_cond = 0;
	while(!exit_cond){
		if(pin_irq_configured[PD][i] == 1){
			if((PORTD->PCR[i] & PORT_PCR_ISF_MASK) == PORT_PCR_ISF_MASK){
				PORTD->PCR[i] |= PORT_PCR_ISF_MASK;
				port_pin_interrupt[PD][i]();
				exit_cond = 1;
			}
		}
		i++;
		if(i == 32){
			exit_cond = 1;
		}
	}
}
__ISR__ PORTE_IRQHandler(void){
	uint32_t i = 0;
	uint32_t exit_cond = 0;
	while(!exit_cond){
		if(pin_irq_configured[PE][i] == 1){
			if((PORTE->PCR[i] & PORT_PCR_ISF_MASK) == PORT_PCR_ISF_MASK){
				PORTE->PCR[i] |= PORT_PCR_ISF_MASK;
				port_pin_interrupt[PE][i]();
				exit_cond = 1;
			}
		}
		i++;
		if(i == 32){
			exit_cond = 1;
		}
	}
}

