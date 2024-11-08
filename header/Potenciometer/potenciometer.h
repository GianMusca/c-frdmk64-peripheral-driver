/***************************************************************************//**
  @file     potenciometro.h
  @brief    TEST ONLY - Potenciometer interface
  @author   Tobias Demeco
 ******************************************************************************/

#ifndef _POTE_H_
#define _POTE_H_

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

typedef uint16_t pote_t;

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief initializes potenciometer
 * @return nothing
 */
void pote_init(void);

/**
 * @brief reads value of potenciometer
 * @return current value of potenciometer
 */
pote_t pote_read(void);

/**
 * @brief changes value of virtual potenciometer (TEST ONLY)
 * @param newValue new value of potenciometer
 * @return nothing
 */
void pote_write(pote_t newValue);
/*******************************************************************************
 ******************************************************************************/

#endif // _POTE_H_