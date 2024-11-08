/***************************************************************************//**
  @file     +Nombre del archivo (ej: template.c)+
  @brief    +Descripcion del archivo+
  @author   +Nombre del autor (ej: Salvador Allende)+
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "board.h"
#include "gpio.h"
#include "SPI.h"
#include "SPI_joystick.h"
#include "timer.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define MAX_JOYSTICKS		1
#define LEN(array) (sizeof(array) / sizeof(array[0]))
#define MAX_FRAMES 5


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
typedef struct {
	joystickID id;
	SPI_id spi;
	bool enable;			//It can be declared and left momentarily disabled
	tim_id_t timer;
	SPI_Joystick_Events eventQueue[MAX_SPI_JOYSTICK_QUEUE];
	uint8_t eventBeginIndex;
	uint8_t eventEndIndex;
} SPI_joystick_t;


/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/
static SPI_joystick_t joystickArray[MAX_JOYSTICKS];


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/
bool isValidJoystickID(joystickID id);
void spiJoystickUpdate(void);
void appendSpiJoystickEvent(joystickID id, SPI_Joystick_Events event);
void sortSpiJoystickEvent(joystickID id, uint8_t frame);


/*void read_test(joystickID id, uint16_t* buff);
static int dummy[MAX_FRAMES] = {1,64,65,9};*/
/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

// +ej: static const int temperaturas_medias[4] = {23, 26, 24, 29};+


/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/
uint8_t bit_R = 0;
uint8_t bit_L = 0;
uint8_t bit_P = 0;


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
/**
 * @brief Initializes the joystick driver
 */
void spiJoystickInit(uint8_t R,uint8_t L,uint8_t P)
{
	uint8_t i;
	timerInit();
	SPI_cfg_t cfg = { .baudrate= 1220,
			.endiannes = MSB,
			.frame_size = 8,
			.mode = MASTER,
			.pcs_polarity = INACTIVE_H,
			.sck_phase = CPHA_LOW,
			.sck_polarity = SCK_LOW	};
	SPI_init(SPI_0, cfg);
	bit_R = R;
	bit_L = L;
	bit_P = P;
	for(i=0; i<MAX_JOYSTICKS; i++){joystickArray[i].id = -1;}
}

/**
 * @brief create a new encoder structure
 * @param id: The SPI use by the new joystick
 * @return ID for a new encoder, will return -1 if it couldn't create new joystick;
 */
joystickID getSpiJoystickID()
{
	joystickID id = -1;
	tim_id_t timer;
	for(uint8_t i=0; i<MAX_JOYSTICKS; i++)
		if(joystickArray[i].id == -1){					//If there is an available free joystick, then register it
			id = i;
			joystickArray[i].id = i;
			joystickArray[i].spi = SPI_0;
			joystickArray[i].enable = true;
			timer = timerGetId();
			joystickArray[i].timer = timer;
			timerStart(joystickArray[i].timer, (uint32_t)TIMER_MS2TICKS(SPI_JOYSTICK_REFRESH_PERIOD), TIM_MODE_PERIODIC, spiJoystickUpdate); //start individual timer
			joystickArray[i].eventBeginIndex = 0;
			joystickArray[i].eventEndIndex = 0;

			for(uint8_t j=0; j< MAX_SPI_JOYSTICK_QUEUE; j++)
				joystickArray[i].eventQueue[j] = END_OF_SPI_JOYSTICK_QUEUE;
			break;
		}

	return id;
}


/**
 * @brief Read the oldest event for a given joystick, the user must go through the queue if he needs it, until END_OF_QUEUE reached
 * @param id: the id of the given joystick
 * @return oldest event in the event queue, will return INVALID if the id corresponds to a disabled or invalid joystick
 */
const SPI_Joystick_Events getSpiJoystickEvent(joystickID id)
{
	SPI_Joystick_Events ret;
	if(isValidJoystickID(id))
	{
		if (joystickArray[id].eventBeginIndex != joystickArray[id].eventEndIndex)
		{
			ret = joystickArray[id].eventQueue[joystickArray[id].eventBeginIndex];
			joystickArray[id].eventBeginIndex = (++joystickArray[id].eventBeginIndex) % (LEN(joystickArray[id].eventQueue));
		}

		else
			ret = END_OF_SPI_JOYSTICK_QUEUE;
	}

	return ret;
}




/**
 * @brief Disables a given joystick
 * @param id: the id of the given joystick
 */
void disableSpiJoystick(joystickID id)
{
	if(isValidJoystickID(id))
		joystickArray[id].enable = false;
}


/**
 * @brief Enables a given joystick
 * @param id: the id of the given joystick
 */
void enableSpiJoystick(joystickID id)
{
	if(isValidJoystickID(id))
		joystickArray[id].enable = true;
}


/**
 * @brief Informs on whether or not a given encoder is joystick
 * @param id: the id of the given joystick
 * @return TRUE if a joystick is enabled, FALSE if the joystick deesn't exist or is disabled
 */
bool isSpiJoystickEnabled(joystickID id)
{
	bool ret = false;
	if(isValidJoystickID(id))
		ret = joystickArray[id].enable;
	return ret;
}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
/**
 * @brief validates encoder ID
 * @param id: the id of the given encoder
 * @return either TRUE or FALSE
 */
bool isValidJoystickID(joystickID id)
{
	bool valid = false;
	uint8_t i;
	for(i=0; i<MAX_JOYSTICKS; i++){
		if(joystickArray[i].id == id)
			valid = true;
	}
	return valid;
}


/**
 * @brief Updates buttons events with timers
 */
void spiJoystickUpdate(void){
	uint8_t i;
	for(i=0; i < MAX_JOYSTICKS; i++){				//Go through every encoder
		if(joystickArray[i].enable == true)//If it's enabled and it's timer expired
		{
			uint16_t buff[MAX_FRAMES];
			for(uint8_t j=0; /*!(SPI_RX_empty(SPI_0)) ||*/ j < MAX_FRAMES; j++){
				//SPI_read(i, (buff+j));
				read_test(i,buff+j);
				sortSpiJoystickEvent(i, buff[j]);
			}

		}
	}
	return;
}

/**
* @brief Queue a new event on the given encoder
* @param id: the id of the given encoder
* @param event: the event that needs to be queued
*/
void appendSpiJoystickEvent(joystickID id, SPI_Joystick_Events event){
	if(isValidJoystickID(id)){
		if (joystickArray[id].eventEndIndex != (joystickArray[id].eventBeginIndex + LEN(joystickArray[id].eventQueue)))
			{
				joystickArray[id].eventQueue[joystickArray[id].eventEndIndex] = event;
				joystickArray[id].eventEndIndex = (++joystickArray[id].eventEndIndex) % (LEN(joystickArray[id].eventQueue));
			}
	}
}

void sortSpiJoystickEvent(joystickID id,uint8_t frame)
{
	if((1<<bit_R & frame))
	{
		appendSpiJoystickEvent(id,SPI_JOYSTICK_R_PRESS);
	}

	if((1<<bit_L & frame))
	{
		appendSpiJoystickEvent(id,SPI_JOYSTICK_L_PRESS);
	}

	if((1<<bit_P & frame))
	{
		appendSpiJoystickEvent(id,SPI_JOYSTICK_P_PRESS);
	}

	else{appendSpiJoystickEvent(id,NO_JOYSTICK_EVENT);}

}

/*void read_test(joystickID id, uint16_t* buff)
{
	static uint8_t i = 0;
	buff[0] = dummy[i];
	i++;
}*/
