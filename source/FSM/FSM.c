/***************************************************************************//**
  @file     FSM.c
  @brief    State processing
  @author   Tobias Demeco
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/  

#include "header/FSM/FSM.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

State_Type* fsm(State_Type *p_table , System_Events event){ // Puntero al estado actual , Evento recibido
  //printf(" >>%c<<\n ",event);         // just for test (debug)
  
  // 1-Recorremos las tabla de estado ( Ej estado_0) hasta encontrar el arco que contenga el evento actual 
  while (p_table -> event != event && p_table -> event != EOT)
    p_table++;
  
  if(p_table != NULL)
  {
    (*p_table -> p_action) (&p_table);        /*2- Ejecuta Rutina de accion corresondiente*/
  }

  p_table=p_table -> next_state;      /*3-Encuentro próximo estado*/

  return(p_table);
}
