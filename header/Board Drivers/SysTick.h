/***************************************************************************//**
  @file     SysTick.h
  @brief    SysTick driver
  @author   Nicol�s Magliola
 ******************************************************************************/

#ifndef _SYSTICK_H_
#define _SYSTICK_H_

// VERSION 1MS

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include "hardware.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
typedef void (*systick_callback_t)(void);		//Just a renaming of pointers to functions with void input and output

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Initialize SysTic driver
 * @param irqs_callback, array of Callback interrupts to be called every SysTick
 * @param n, amount of elements in irqs_callback[]
 * @return Initialization and registration succeed
 */
//bool SysTick_Init (void (*funcallback)(void));
bool SysTick_Init (systick_callback_t irqs_callback);


/*******************************************************************************
 ******************************************************************************/

#endif // _SYSTICK_H_
