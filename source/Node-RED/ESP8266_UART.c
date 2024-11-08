/***************************************************************************//**
  @file     ESP8266_UART.c
  @brief    Application functions
  @author   Ale
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "UART.h"
#include "ESP8266_UART.h"
#include "ESP8266_codes.h"
#include "timer.h"

/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

//UART CONFIG
#define ESP_ID		UART_3
#define ESP_BAUD	9600
//OTHER CONFIG
#define BUFF_LENGTH			1000
#define ESP_REFRESH_PERIOD	10		//10mseg of refresh period for receiving new data

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * VARIABLE PROTOTYPES WITH GLOBAL LEVEL SCOPE
 ******************************************************************************/
bool new_data;								//Informs on whether or not there is new data from the ESP
char msg_buffer[BUFF_LENGTH][DATA_LENGTH];	//Data array, where each DATA element has a DATA_LENGTH length
uint32_t oldest_msg = 0;
uint32_t latest_msg = 0;
/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/
void ESP_update();
/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
/**
 * @brief initializes ESP8266 UART communication, assumes UART_3, 9600 baud rate
 * @return
 */
void ESP8266_UART_init(void){
	UART_init(ESP_ID, ESP_BAUD, UART_NON_BLOCKING, UART_NO_PARITY, NBITS_8);
	new_data = false;

	timerInit();			//Just in case our user hasn't already initialized the timer driver
	tim_id_t ESP_timer = timerGetId();
	timerStart(ESP_timer, TIMER_MS2TICKS(ESP_REFRESH_PERIOD), TIM_MODE_PERIODIC, ESP_update);	//Polling of sorts
}

/**
 * @brief resolves communications from the ESP8266 and copies useful data
 * @param data structure where the data will be copied
 * @return
 */
void ESP8266_parse_msg(ESP_data* data){
	switch(msg_buffer[oldest_msg][0]){
		case ADD_HEADER:		data->new_pixel = true;		break;
		case DESTROY_HEADER:	data->destroy_pixel = true;	break;
		case BRIGHTNESS_HEADER:
			data->brightness = (uint8_t)msg_buffer[oldest_msg][1];
			break;
		case COLOR_HEADER:
			data->rgb.r = (uint8_t)msg_buffer[oldest_msg][1];
			data->rgb.g = (uint8_t)msg_buffer[oldest_msg][2];
			data->rgb.b = (uint8_t)msg_buffer[oldest_msg][3];
			break;
	}
	//point to next data in the circular buffer
	if(oldest_msg != latest_msg){				//If oldest and latest are already the same, we don't need to move them
		if(oldest_msg + 1 == BUFF_LENGTH)
			oldest_msg = 0;
		else
			oldest_msg++;
	}
	else
		new_data = false;		//If oldest is already the same as latest, and we read, now there is no new data
}

/**
 * @brief sends relevant data to the ESP8266
 * @param data structure where the data will be read
 * @return
 */
void ESP8266_send_data(ESP_data* data){
	char msg[TX_LENGTH];

	msg[0] = 'P';
	msg[1] = data->pot >> 8;		//Higher part of the 16 bits
	msg[2] = data->pot & 0x00FF;	//Lower part of the 16 bits
	UART_write(ESP_ID, msg, TX_LENGTH);

	msg[0] = 'X';
	msg[1] = data->accel_x >> 8;		//Higher part of the 16 bits
	msg[2] = data->accel_x & 0x00FF;	//Lower part of the 16 bits
	UART_write(ESP_ID, msg, TX_LENGTH);

	msg[0] = 'Y';
	msg[1] = data->accel_y >> 8;		//Higher part of the 16 bits
	msg[2] = data->accel_y & 0x00FF;	//Lower part of the 16 bits
	UART_write(ESP_ID, msg, TX_LENGTH);

	msg[0] = 'Z';
	msg[1] = data->accel_z >> 8;		//Higher part of the 16 bits
	msg[2] = data->accel_z & 0x00FF;	//Lower part of the 16 bits
	UART_write(ESP_ID, msg, TX_LENGTH);
}

/**
 * @brief informs on new data from the ESP8266
 * @param
 * @return whether or not there is new data from the ESP8266
 */
bool new_ESP_data_available(){
	return new_data;
}

/*******************************************************************************
 *******************************************************************************
                        PRIVATE FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
void ESP_update(){
	uint8_t i;
	static char msg[DATA_LENGTH];
	uint32_t msg_length = UART_get_msg_length(ESP_ID);
	if(UART_is_new_info(ESP_ID) && msg_length > DATA_LENGTH){	//If there is new data on the UART, and there is a DATA_LENGTH amount o bytes to parse
		UART_read(ESP_ID, msg, DATA_LENGTH);					//Transfer msg
		for(i = 0; i<DATA_LENGTH; i++)
			msg_buffer[latest_msg][i] = msg[i];					//Copy msg to our msg buffer

		if(latest_msg + 1 == BUFF_LENGTH)
			latest_msg = 0;										//If circular buffer overflows, reset index
		else
			latest_msg++;

		if(latest_msg == oldest_msg){							//If the indexes meet, we lose the oldest msg, and modify the oldest index
			if(oldest_msg + 1 == BUFF_LENGTH)
				oldest_msg = 0;
			else
				oldest_msg++;
		}
		new_data = true;				//Raise new_data flag
	}
}
/*******************************************************************************
                        INTERRUPT ZONE
 ******************************************************************************/
