/***************************************************************************//**
  @file     FSMTable.c
  @brief    State processing - Matrixes
  @author   Tobias Demeco
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/  
#include "header/FSM/FSMTable.h"
#include "header/FSM/FSM_Callbacks.h"


/*******************************************************************************
 *******************************************************************************
                            STATE MATRIXES DEFINITION
 *******************************************************************************
 ******************************************************************************/

/*** Horizonal moving ***/
State_Type horizontal[]=
{//Event        State           Action
{NO_EVENT,      horizontal,     nothing},      
{PRESSED_L,     horizontal,     move_left},         //cambia a modify entonces debe poner modify en pantalla
{PRESSED_R,     horizontal,     move_right},        //cambia a options entonces debe poner options en pantalla
{PRESSED_P,     vertical,       nothing},           //elige access entonces aparece card en pantalla
{DATA_CHANGE,   horizontal,     update_data},
{NODE_UPDATE,   horizontal,     update_node_ui},
{LED_UPDATE,    horizontal,     update_led},
{EOT,           horizontal,     nothing},
};

/*** Vertical moving ***/
State_Type vertical[]=
{//Event        State           Action
{NO_EVENT,      vertical,       nothing},      
{PRESSED_L,     vertical,       move_up},         //cambia a modify entonces debe poner modify en pantalla
{PRESSED_R,     vertical,       move_down},        //cambia a options entonces debe poner options en pantalla
{PRESSED_P,     horizontal,     nothing},           //elige access entonces aparece card en pantalla
{DATA_CHANGE,   vertical,       update_data},
{NODE_UPDATE,   vertical,       update_node_ui},
{LED_UPDATE,    vertical,       update_led},
{EOT,           vertical,       nothing},
};

////////////////////////////////////////////////////////////////////
