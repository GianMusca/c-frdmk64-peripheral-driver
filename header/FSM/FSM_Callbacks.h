/***************************************************************************//**
  @file     FSM.h
  @brief    //
  @author  Matias Tripoli
 ******************************************************************************/

#ifndef _FSMCALL_H_
#define _FSMCALL_H_

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "FSM.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define VEL_SCALAR 6000          //Position moves 1/6000 of velocity (calibration: gravity should move the pixel from stop to next led in 300ms)
/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES WITH GLOBAL SCOPE
 ******************************************************************************/

/**
 * @brief Does nothing
 */
void nothing(State_Type** p_state);


/*** Movement functions ***/
/**
 * @brief Moves LED up a position
 */
void move_up(State_Type** p_state);

/**
 * @brief Moves LED down a position
 */
void move_down(State_Type** p_state);

/**
 * @brief Moves LED left one position
 */
void move_left(State_Type** p_state);

/**
 * @brief Moves LED right one position
 */
void move_right(State_Type** p_state);


/*** Data updating ***/
/**
 * @brief Updates data obtained from Node-RED
 */
void update_data(State_Type** p_state);

/**
 * @brief Updates node-RED ui with newest data
 */
void update_node_ui(State_Type** p_state);

/**
 * @brief Updates led position and velocity
 */
void update_led(State_Type** p_state);

#endif // _FSMCALL_H_
