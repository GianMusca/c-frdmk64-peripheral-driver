/***************************************************************************//**
  @file     FSM_Callbacks.c
  @brief    Callback functions for each event
  @author   Tobias Demeco
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/  

#include "header/FSM/FSMTable.h"
#include "header/FSM/FSM_Callbacks.h"

#include "header/Event Handler/event_handler.h"
#include "header/Node-RED/ESP8266_UART.h"
#include "header/Potenciometer/potenciometer.h"
#include "header/Accelerometer/FXOS8700CQ.h"
#include "header/LED Matrix/LED_matrix_app.h"



/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
typedef struct{
	int32_t velX;
	int32_t velY;
} velocity_t;

typedef struct{
    uint8_t row;
    uint8_t col;
} led_pos_t;
/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
 * CALLBACK VARIABLES
 ******************************************************************************/
static ESP_data ledData = {255, 0, 0, false, false, 50, 50, 0, 0, 0};	   //led data (color, brightness, acceleration)
static led_pos_t ledPosition = {3, 3};				    //led position
static velocity_t ledVelocity = {0, 0};				    //led velocity
static Acceleration_Type ledAccelerations = {0, 0, 0};			//variable for quick access to acceleration values
static uint8_t lastPoteReading = 50;
static uint16_t speed_counter = 0;



/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

//Does nothing
void nothing(State_Type** p_state)
{
    return;
}

void update_node_ui(State_Type** p_state)
{
    ledAccelerations = FXOS8700CQ_get_Acceleration();         //updates accelerations
    ledData.accel_x = ledAccelerations.x;
    ledData.accel_y = ledAccelerations.y;
    ledData.accel_z = ledAccelerations.z;

    ledData.pot = pote_read();                      //updates potenciometer reading

    if(lastPoteReading != ledData.pot)              //if potenciometer has changed since last measurement
    {
        ledData.brightness = ledData.pot;           //update brightness
        lastPoteReading = ledData.pot;              //update last reading with current one for next call
    }

    ESP8266_send_data(&ledData);

    return;
}

void move_up(State_Type** p_state)
{
    if(ledPosition.row > 0)
        ledPosition.row--;
    else
        ledPosition.row = 0;
    return;
}

void move_down(State_Type** p_state)
{
    if(ledPosition.row < 7)
        ledPosition.row++;
    else
        ledPosition.row = 7;
    return;
}


void move_left(State_Type** p_state)
{
    if(ledPosition.col > 0)
        ledPosition.col--;
    else
        ledPosition.row = 0;
    return;
}

void move_right(State_Type** p_state)
{
    if(ledPosition.col < 7)
        ledPosition.col++;
    else
        ledPosition.col = 7;
    return;
}


void update_data(State_Type** p_state)
{
    ESP_data dummy;
    ESP8266_parse_msg(&dummy);

    if(dummy.rgb.r != 0 || dummy.rgb.g != 0 || dummy.rgb.b != 0)
        ledData.rgb = dummy.rgb;
    if(dummy.brightness != 0)
    {
        ledData.brightness = dummy.brightness;
    }
    return;
}

void update_led(void){				//WILL PROBABLY HAVE TO UPDATE THIS FUNCTION (TO ADD A MULTIPLICATIVE FACTOR TO ACCELERATION)

    if(speed_counter < 10)                                  //update led position, velocities and accelerations every 100ms
        speed_counter++;
    else{
        ledPosition.row += ledVelocity.velX/VEL_SCALAR;		//update led row position in matrix
        if(ledPosition.row <= 0)					        //left wall
        {
            ledVelocity.velX = 0;
            ledPosition.row = 0;
        }
        if(ledPosition.row >= 7)					        //right wall
        {
            ledVelocity.velX = 0;
            ledPosition.row = 7;
        }

        ledPosition.col += ledVelocity.velY/VEL_SCALAR;	    //update led column position in matrix
        if(ledPosition.col <= 0)					        //floor
        {
            ledVelocity.velY = 0;
            ledPosition.col = 0;
        }
        if(ledPosition.col >= 7)					        //ceiling
        {
            ledVelocity.velY = 0;
            ledPosition.col = 7;
        }
        ledAccelerations = FXOS8700CQ_get_Acceleration();	//update accelerations
        ledVelocity.velX += ledAccelerations.x;//update led velocities acording to accelerations
        ledVelocity.velY += ledAccelerations.y;
    }

    LED_matrix_app_dot(ledPosition.row, ledPosition.col, ledData.rgb);	//show current led
	LED_matrix_app_brightness(ledData.brightness);								//set current brightness
}