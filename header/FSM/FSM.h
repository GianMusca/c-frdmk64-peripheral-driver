/***************************************************************************//**
  @file     FSM.h
  @brief    //
  @author  Matias Tripoli
 ******************************************************************************/

#ifndef _FSM_H_
#define _FSM_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "header/Event Handler/event_handler.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

//EVENTS
#define EOT -1

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/


typedef struct state_diagram_edge State_Type;
struct state_diagram_edge
{
  System_Events event;
  State_Type *next_state;
  void (*p_action)(State_Type** p_table);
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
 * @param mode INPUT, OUTPUT, INPUT_PULLUP or INPUT_PULLDOWN.
 */

State_Type* fsm(State_Type *p_table , System_Events event);

/*******************************************************************************
 ******************************************************************************/

#endif // _FSM_H_
