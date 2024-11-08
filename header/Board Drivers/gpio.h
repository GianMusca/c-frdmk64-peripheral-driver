/***************************************************************************//**
  @file     gpio.h
  @brief    Simple GPIO Pin services, similar to Arduino
  @author   Nicolás Magliola
 ******************************************************************************/

#ifndef _GPIO_H_
#define _GPIO_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

//Alternatives
typedef enum{
	PORT_mAnalog,
	PORT_mGPIO,
	PORT_mAlt2,
	PORT_mAlt3,
	PORT_mAlt4,
	PORT_mAlt5,
	PORT_mAlt6,
	PORT_mAlt7,
} PORTMux_t;			//See Kamman, page 10

// Ports
enum { PA, PB, PC, PD, PE };

// Convert port and number into pin ID
// Ex: PTB5  -> PORTNUM2PIN(PB,5)  -> 0x25
//     PTC22 -> PORTNUM2PIN(PC,22) -> 0x56
#define PORTNUM2PIN(p,n)    (((p)<<5) + (n))
#define PIN2PORT(p)         (((p)>>5) & 0x07)
#define PIN2NUM(p)          ((p) & 0x1F)


// Modes
#ifndef INPUT
#define INPUT               0
#define OUTPUT              1
#define INPUT_PULLUP        2
#define INPUT_PULLDOWN      3
#endif // INPUT

//DSE MODES
#define DSE_DEFAULT			0
#define DSE_HIGH_STRENGTH	1
#define DSE_LOW_STRENGTH	2

//ODE MODES
#define ODE_DEFAULT			0
#define ODE_ENABLE			1
#define ODE_DISABLE			2

//SRE MODES
#define SRE_DEFAULT			0
#define SRE_SLOW			1
#define SRE_FAST			2

//PFE MODES
#define PFE_DEFAULT			0
#define PFE_ENABLE			1
#define PFE_DISABLE			2

//DFE MODES
#define DFE_DEFAULT			0
#define DFE_ENABLE			1
#define DFE_DISABLE			2



// Digital values
#ifndef LOW
#define LOW     0
#define HIGH    1
#endif


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef uint8_t pin_t;
typedef void (*pinIRQFunction_t)(void);

// IRQC modes, see page 289 of K64 Reference Manual for complete scheme
enum {
    GPIO_IRQ_MODE_DISABLE 		= 0,	//0000	Interrupt Status Flag (ISF) is disabled
	GPIO_IRQ_MODE_LOW_STATE 	= 8,	//1000	ISF flag and Interrupt when logic 0
    GPIO_IRQ_MODE_RISING_EDGE 	= 9,	//1001	ISF flag and Interrupt on rising-edge
    GPIO_IRQ_MODE_FALLING_EDGE 	= 10,	//1010	ISF flag and Interrupt on falling-edge
    GPIO_IRQ_MODE_BOTH_EDGES 	= 11,	//1011	ISF flag and Interrupt on either edge
	GPIO_IRQ_MODE_HIGH_STATE 	= 12,	//1100	ISF flag and Interrupt when logic 1
    GPIO_IRQ_CANT_MODES
};

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

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
void gpioMode (pin_t pin, uint8_t mode, uint8_t DSE_mode, uint8_t ODE_mode, uint8_t SRE_mode, uint8_t PFE_mode, uint8_t DFE_mode, uint8_t DFW_width);

/**
 * @brief Write a HIGH or a LOW value to a digital pin
 * @param pin the pin to write (according PORTNUM2PIN)
 * @param val Desired value (HIGH or LOW)
 */
void gpioWrite (pin_t pin, bool value);

/**
 * @brief Toggle the value of a digital pin (HIGH<->LOW)
 * @param pin the pin to toggle (according PORTNUM2PIN)
 */
void gpioToggle (pin_t pin);

/**
 * @brief Reads the value from a specified digital pin, either HIGH or LOW.
 * @param pin the pin to read (according PORTNUM2PIN)
 * @return HIGH or LOW
 */
bool gpioRead (pin_t pin);

/**
 * @brief Enables Interrupts at IRQn port.
 * @param IRQn: port where interrupts will be enabled.
 */
//static void interrupt_init(uint32_t IRQn);

/**
 * @brief Configures pin to IRQ events
 * @param pin: pin to set IRQ, use NUM2PIN
 * @param IRQMode: see IRQC modes enum definition at top of file
 * @param IRQFunction: function to call at pin interrupt triggered
 * @return validation of config
 */
bool gpioIRQ (pin_t pin, uint8_t IRQMode, pinIRQFunction_t IRQFun);


/*******************************************************************************
 ******************************************************************************/

#endif // _GPIO_H_

