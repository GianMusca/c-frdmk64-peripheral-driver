/***************************************************************************//**
  @file     timer.c
  @brief    Timer driver. Advance implementation
  @author   Nicolás Magliola
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/

#include "timer.h"

#include "SysTick.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/

//#if TIMER_TICK_MS != (1000U/SYSTICK_ISR_FREQUENCY_HZ)
//#error Las frecuencias no coinciden!!
//#endif // TIMER_TICK_MS != (1000U/SYSTICK_ISR_FREQUENCY_HZ)

#define TIMER_DEVELOPMENT_MODE    1

#define TIMER_ID_INTERNAL   0


/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/

typedef struct {
	ttick_t             period;
	ttick_t             cnt;
    tim_callback_t      callback;
    uint8_t             mode        : 1;
    uint8_t             running     : 1;
    uint8_t             expired     : 1;
    uint8_t             unused      : 5;
} timer_t;


/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/

/**
 * @brief Periodic service
 */
static void timer_isr(void);


/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/

static timer_t timers[TIMERS_MAX_CANT];
static tim_id_t timers_cant = TIMER_ID_INTERNAL+1;


/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

void timerInit(void)
{
    static bool yaInit = false;
    if (yaInit)
        return;

    timers[TIMER_ID_INTERNAL].running = false;

    SysTick_Init(timer_isr); // init peripheral

    yaInit = true;
}


tim_id_t timerGetId(void)
{
#ifdef TIMER_DEVELOPMENT_MODE
    if (timers_cant >= TIMERS_MAX_CANT)
    {
        return TIMER_INVALID_ID;
    }
    else
#endif // TIMER_DEVELOPMENT_MODE
    {
    	tim_id_t newId = timers_cant;
    	timers_cant ++;
    	timers[newId].running = false;
        return newId;
    }
}


// todo: preguntar si disable con running
// todo: razon de macros ms2ticks? (en q parte deberia colocarlas)

void timerStart(tim_id_t id, ttick_t ticks, uint8_t mode, tim_callback_t callback)
{
#ifdef TIMER_DEVELOPMENT_MODE
    if ((id < timers_cant) && (mode < CANT_TIM_MODES))
#endif // TIMER_DEVELOPMENT_MODE
    {
        // ****** COMPLETAR ******
        // disable timer
        // configure timer
        // enable timer

    	timers[id].running = false;

    	timers[id].callback = callback;
    	timers[id].cnt = 0;
    	timers[id].expired = false;
    	timers[id].mode = mode;
    	timers[id].period = ticks;

    	timers[id].running = true;
    }
}


void timerStop(tim_id_t id)
{
    // ****** COMPLETAR ******
    // Apago el timer
    // y bajo el flag

	timers[id].running = false;
	timers[id].expired = true;
}


bool timerExpired(tim_id_t id)
{
    // ****** COMPLETAR ******
    // Verifico si expiró el timer
    // y bajo el flag

	return timers[id].expired;
}


void timerDelay(ttick_t ticks)
{
    timerStart(TIMER_ID_INTERNAL, ticks, TIM_MODE_SINGLESHOT, NULL);
    while (!timerExpired(TIMER_ID_INTERNAL))
    {
        // wait...
    }
}


/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/

static void timer_isr(void)
{


    // ****** COMPLETAR ******
    // decremento los timers activos
    // si hubo timeout!
    // 1) execute action: callback or set flag
    // 2) update state


	unsigned int id;
	for(id = 0; id<timers_cant && id<TIMERS_MAX_CANT;id++){
		if (timers[id].running){
			timers[id].cnt++;
			if (timers[id].cnt >= timers[id].period){
				if(timers[id].callback != NULL)
					timers[id].callback();
				timers[id].cnt = 0;
				if(timers[id].mode == TIM_MODE_SINGLESHOT){
					timers[id].running = false;
					timers[id].expired = true;
				}
			}
		}
	}


}


/******************************************************************************/

