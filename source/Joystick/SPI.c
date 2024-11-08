/***************************************************************************//**
  @file     +Nombre del archivo (ej: template.c)+
  @brief    +Descripcion del archivo+
  @author   +Nombre del autor (ej: Salvador Allende)+
 ******************************************************************************/

/*******************************************************************************
 * INCLUDE HEADER FILES
 ******************************************************************************/
#include "SPI.h"
#include "../CMSIS/MK64F12.h"
#include "hardware.h"
#include "board.h"


/*******************************************************************************
 * CONSTANT AND MACRO DEFINITIONS USING #DEFINE
 ******************************************************************************/
#define SPI_MUX_ALT 2

//Frame sizes
#define FMSZ_MIN 4
#define FMSZ_DEFAULT 8
#define FMSZ_MAX 16

//Baud rate
#define BDRT_MIN 436
#define BDRT_DEFAULT 1220
#define BDRT_MAX 50000000

#define SPI_CLOCK_FREQUENCY 1000000

#define ENABLE 0
#define DISABLE 1

/*******************************************************************************
 * ENUMERATIONS AND STRUCTURES AND TYPEDEFS
 ******************************************************************************/
typedef enum{
       CTAR_0,
       CTAR_1
} CTAR_id;

typedef struct{
	uint8_t cont  :1;
	uint8_t ctas  :1;
	uint8_t eoq   :1;
	uint8_t ctcnt :1;
	uint8_t pcs   :6;
	uint16_t data;
}SPI_pushr_cfg_t;
/*******************************************************************************
 * VARIABLES WITH GLOBAL SCOPE
 ******************************************************************************/

// +ej: unsigned int anio_actual;+


/*******************************************************************************
 * FUNCTION PROTOTYPES FOR PRIVATE FUNCTIONS WITH FILE LEVEL SCOPE
 ******************************************************************************/
static void SPI_clock_gating(SPI_id id);
static void SPI_port_muxing(SPI_id id);
static void SPI_mode(SPI_id id, SPI_mode_t mode);
static void SPI_PCS_inactive_state(SPI_id id, SPI_SS_polarity pcs);
static void SPI_FIFO_enable(SPI_id id, uint8_t tx_en, uint8_t rx_en);
static void SPI_frame_size(SPI_id, CTAR_id nctar , uint8_t fsize);
static void SPI_clock_polarity(SPI_id id, CTAR_id nctar, SPI_clock_polarity_t cpol);
static void SPI_clock_phase(SPI_id id, CTAR_id nctar, SPI_clock_phase_t cpha);
static void SPI_LSB(SPI_id id, CTAR_id nctar,SPI_endiannes bf);
static void SPI_baudrate(SPI_id id, uint32_t baudrate);

static void SPI_clean_flags(SPI_id id);
static void SPI_IRQ_enable(SPI_id id);
static void SPI_final_enable(SPI_id id);

static void SPI_write_frame(SPI_id id, SPI_pushr_cfg_t cfg);

/*******************************************************************************
 * ROM CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/
static SPI_Type * SPI_PTRS[] = SPI_BASE_PTRS;
static uint8_t SPI_IRQS_arr[] = SPI_IRQS;


/*******************************************************************************
 * STATIC VARIABLES AND CONST VARIABLES WITH FILE LEVEL SCOPE
 ******************************************************************************/
static uint8_t SPI_PCS0_NUM[] = {PORTNUM2PIN(PD,0), PORTNUM2PIN(PB,3), PORTNUM2PIN(PB, 20)};
static uint8_t SPI_PCS1_NUM[] = {PORTNUM2PIN(PC,3), PORTNUM2PIN(PB,9), PORTNUM2PIN(PD, 15)};
static uint8_t SPI_PCS2_NUM[] = {PORTNUM2PIN(PC,2), PORTNUM2PIN(PE,5), PORTNUM2PIN(PD, 15)};
static uint8_t SPI_PCS3_NUM[] = {PORTNUM2PIN(PC,1), PORTNUM2PIN(PE,6), PORTNUM2PIN(PD, 15)};
static uint8_t SPI_PCS4_NUM[] = {PORTNUM2PIN(PC,0), PORTNUM2PIN(PE,6), PORTNUM2PIN(PD, 15)};
static uint8_t SPI_PCS5_NUM[] = {PORTNUM2PIN(PB,23), PORTNUM2PIN(PE,6), PORTNUM2PIN(PD, 15)};
static uint8_t SPI_SIN_NUM[]  = {PORTNUM2PIN(PD,3), PORTNUM2PIN(PB,17), PORTNUM2PIN(PB, 23)};
static uint8_t SPI_SOUT_NUM[] = {PORTNUM2PIN(PD,2), PORTNUM2PIN(PB,16), PORTNUM2PIN(PB, 22)};
static uint8_t SPI_SCK_NUM[]  = {PORTNUM2PIN(PD,1), PORTNUM2PIN(PB,2), PORTNUM2PIN(PB, 21)};;


//Table SPI Prescaler
static uint8_t prescaler_table[] = {
  2,
  3,
  5,
  7
};

//Table SPI Scaler
static uint16_t scaler_table[] = {
  2,
  4,
  6,
  8,
  16,
  32,
  64,
  128,
  256,
  512,
  1024,
  2048,
  4096,
  8192,
  16384,
  32768
};




/*******************************************************************************
 *******************************************************************************
                        GLOBAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
void SPI_init(SPI_id id, SPI_cfg_t cfg)
{
	//CLock Gating
	SPI_clock_gating(id);

	//GPIO Mux
	SPI_port_muxing(id);

	//SPI configuration
	SPI_PTRS[id]->MCR |= SPI_MCR_HALT(1);
	SPI_PTRS[id]->MCR |= SPI_MCR_DCONF(0);

	//Master or Slave mode
	SPI_mode(id,cfg.mode);

	//FIFO overflow
	SPI_PTRS[id]->MCR |= SPI_MCR_ROOE(0); //Ignored=>0  Shifted=>1

	//Inactive state of PCSx
	SPI_PCS_inactive_state(id, cfg.pcs_polarity);

	//FIFO enable/disable
	SPI_FIFO_enable(id, ENABLE, ENABLE);

	CTAR_id nctar = CTAR_0;
	//Set Frame Size
	SPI_frame_size(id, nctar, cfg.frame_size);

	//Clock polarity
	SPI_clock_polarity(id, nctar, cfg.pcs_polarity);

	//Clock phase
	SPI_clock_phase(id, nctar, cfg.sck_phase);

	//LSB or MSB first
	SPI_LSB(id, nctar, cfg.endiannes);

	if(cfg.mode == MASTER)
	{
		SPI_baudrate(id, cfg.baudrate);
	}

	//Clean flags
	SPI_clean_flags(id);

	//Enable interrupts
	//SPI_IRQ_enable(id);

	//Enable SPI Module
	SPI_final_enable(id);

}

void SPI_read(SPI_id id, uint16_t* buff)
{
	if (SPI_PTRS[id]->SR & SPI_SR_RXCTR_MASK)
	{
		SPI_PTRS[id]->SR |= SPI_SR_TCF(1); //Clears TCF bit

	    while (!SPI_read_done(SPI_0))
	    {
	    	//Waits for on going tansfer to end
	    }

		buff[0] = SPI_PTRS[id]->POPR;
	}

}

bool SPI_read_done(SPI_id id)
{
	SPI_PTRS[id]->SR |= SPI_SR_TCF(1);
	bool done = SPI_PTRS[id]->SR & SPI_SR_TCF_MASK;
	return done;
}


bool SPI_RX_full(SPI_id id)
{
	SPI_PTRS[id]->SR |= SPI_SR_RFOF(1);
	bool done = SPI_PTRS[id]->SR & SPI_SR_RFOF_MASK ;
	return done;
}

bool SPI_RX_empty(SPI_id id)
{
	SPI_PTRS[id]->SR |= SPI_SR_RFDF(1);
 	bool done = SPI_PTRS[id]->SR & SPI_SR_RFDF_MASK ;
	return done;
}

bool SPI_write(SPI_id id, SPI_slave slave, const uint16_t* msg, uint32_t length)
{
	SPI_pushr_cfg_t cfg;

	//if(SPI_TX_full(id)){return false;} //Cant write, FIFO is full

	for(uint8_t i=0; i < length; i++)
	{
		if((i+1)==length)
		{
			cfg.cont = true;
			cfg.ctas = CTAR_0;
			cfg.ctcnt = false;
			cfg.eoq = false;
			cfg.pcs = slave;
			cfg.data = msg[i];
		}

		else
		{
			cfg.cont = false;
			cfg.ctas = CTAR_0;
			cfg.ctcnt = false;
			cfg.eoq = true;
			cfg.pcs = slave;
			cfg.data = msg[i];
		}

		SPI_PTRS[0]->MCR =(SPI_PTRS[0]->MCR & (~SPI_MCR_HALT_MASK))| SPI_MCR_HALT(1);
		SPI_PTRS[0]->SR=(SPI_PTRS[0]->SR & (~SPI_SR_TCF_MASK) ) | SPI_SR_TCF(1);
		SPI_write_frame(id,cfg);
		SPI_PTRS[0]->MCR =(SPI_PTRS[0]->MCR & (~SPI_MCR_HALT_MASK))| SPI_MCR_HALT(0);

		while(!(SPI_PTRS[0]->SR & SPI_SR_TCF_MASK));//espero a que se envie el frame
	}



}

bool SPI_write_done(SPI_id id)
{
	SPI_PTRS[id]->SR |= SPI_SR_TCF(1);
	bool done = SPI_PTRS[id]->SR & SPI_SR_TCF_MASK ;
	return done;
}

bool SPI_TX_full(SPI_id id)
{
	SPI_PTRS[id]->SR |= SPI_SR_TFFF(1);
	bool done = ~(SPI_PTRS[id]->SR & SPI_SR_TFFF_MASK);
	return done;
}

bool SPI_TX_empty(SPI_id id)
{
	SPI_PTRS[id]->SR |= SPI_SR_EOQF(1);
	bool done = SPI_PTRS[id]->SR & SPI_SR_EOQF_MASK ;
	return done;
}



/*******************************************************************************
 *******************************************************************************
                        LOCAL FUNCTION DEFINITIONS
 *******************************************************************************
 ******************************************************************************/
static void SPI_clock_gating(SPI_id id)
{
	SIM_Type *sim = SIM;
		switch(id){
			case SPI_0:
				sim->SCGC6 |= SIM_SCGC6_SPI0(1);
				sim->SCGC5 |= SIM_SCGC5_PORTA(1);
				sim->SCGC5 |= SIM_SCGC5_PORTC(1);
				sim->SCGC5 |= SIM_SCGC5_PORTD(1);
				break;

			case SPI_1:
				sim->SCGC6 |= SIM_SCGC6_SPI1(1);
				sim->SCGC5 |= SIM_SCGC5_PORTB(1);
				sim->SCGC5 |= SIM_SCGC5_PORTE(1);
				break;

			case SPI_2:
				sim->SCGC3 |= SIM_SCGC3_SPI2(1);
				sim->SCGC5 |= SIM_SCGC5_PORTB(1);
				sim->SCGC5 |= SIM_SCGC5_PORTD(1);
				break;
		}
}

static void SPI_port_muxing(SPI_id id)
{
	PORT_Type *PORT_PTRS[] = PORT_BASE_PTRS;

	PORT_PTRS[PIN2PORT(SPI_PCS0_NUM[id])]->PCR[PIN2NUM(SPI_PCS0_NUM[id])] = PORT_PCR_MUX(SPI_MUX_ALT);
	PORT_PTRS[PIN2PORT(SPI_PCS0_NUM[id])]->PCR[PIN2NUM(SPI_PCS1_NUM[id])] = PORT_PCR_MUX(SPI_MUX_ALT);
	PORT_PTRS[PIN2PORT(SPI_PCS0_NUM[id])]->PCR[PIN2NUM(SPI_SOUT_NUM[id])] = PORT_PCR_MUX(SPI_MUX_ALT);
	PORT_PTRS[PIN2PORT(SPI_PCS0_NUM[id])]->PCR[PIN2NUM(SPI_SIN_NUM[id])] = PORT_PCR_MUX(SPI_MUX_ALT);
	PORT_PTRS[PIN2PORT(SPI_PCS0_NUM[id])]->PCR[PIN2NUM(SPI_SCK_NUM[id])] = PORT_PCR_MUX(SPI_MUX_ALT);

	if(id == SPI_0 || id == SPI_1)
	{
		PORT_PTRS[PIN2PORT(SPI_PCS0_NUM[id])]->PCR[PIN2NUM(SPI_PCS2_NUM[id])] = PORT_PCR_MUX(SPI_MUX_ALT);
		PORT_PTRS[PIN2PORT(SPI_PCS0_NUM[id])]->PCR[PIN2NUM(SPI_PCS3_NUM[id])] = PORT_PCR_MUX(SPI_MUX_ALT);
	}

	if(id == SPI_0)
	{
		PORT_PTRS[PIN2PORT(SPI_PCS0_NUM[id])]->PCR[PIN2NUM(SPI_PCS4_NUM[id])] = PORT_PCR_MUX(SPI_MUX_ALT);
		PORT_PTRS[PIN2PORT(SPI_PCS0_NUM[id])]->PCR[PIN2NUM(SPI_PCS5_NUM[id])] = PORT_PCR_MUX(SPI_MUX_ALT);
	}

}

static void SPI_mode(SPI_id id, SPI_mode_t mode)
{
	SPI_PTRS[id]->MCR |= SPI_MCR_MSTR(mode);
}


static void SPI_PCS_inactive_state(SPI_id id, SPI_SS_polarity pcs)
{
	SPI_PTRS[id]->MCR |= SPI_MCR_PCSIS(pcs);
}

static void SPI_FIFO_enable(SPI_id id, uint8_t tx_en, uint8_t rx_en)
{
	SPI_PTRS[id]->MCR |= SPI_MCR_DIS_TXF(tx_en);
	SPI_PTRS[id]->MCR |= SPI_MCR_DIS_RXF(rx_en);
}
 
static void SPI_baudrate(SPI_id id, uint32_t baudrate)
{
	uint8_t DBR;
	uint8_t BR;
	uint8_t PBR;

	if(baudrate <= BDRT_MIN || baudrate >= BDRT_MAX)
		baudrate = BDRT_DEFAULT;

	  uint32_t bestBaudRate = 0;
	  uint32_t bestError = 0;
	  uint32_t currentBaudRate = 0;
	  uint32_t currentError = 0;

	  for (uint8_t dbr = 0 ; DBR < 1 ; dbr++)
	  {
	    for (uint8_t pbr = 0 ; pbr < 4 ; pbr++)
	    {
	      for (uint8_t br = 0 ; br < 16 ; br++)
	      {
	        currentBaudRate = (SPI_CLOCK_FREQUENCY * (1 + dbr) ) / (scaler_table[br] * prescaler_table[pbr]);
	        currentError = baudrate < currentBaudRate ? currentBaudRate - baudrate : baudrate - currentBaudRate;
	        if (bestBaudRate == 0 || currentError < bestError)
	        {
	          // Save the current best baud rate values
	          bestBaudRate = currentBaudRate;
	          bestError = currentError;

	          DBR= dbr;
	          BR= br;
	          PBR= pbr;

	        }
	      }
	    }
	  }


	SPI_PTRS[id]->CTAR[0] |= SPI_CTAR_DBR(DBR);
	SPI_PTRS[id]->CTAR[0] |= SPI_CTAR_BR(BR);
	SPI_PTRS[id]->CTAR[0] |= SPI_CTAR_PBR(PBR);

	SPI_PTRS[id]->CTAR[0] |= SPI_CTAR_PASC(PBR) | SPI_CTAR_ASC(BR);
	SPI_PTRS[id]->CTAR[0] |= SPI_CTAR_PDT(PBR) | SPI_CTAR_DT(BR);
}

static void SPI_frame_size(SPI_id id, CTAR_id nctar, uint8_t fsize)
{
	if(fsize < FMSZ_MIN || fsize > FMSZ_MAX)
		fsize = FMSZ_DEFAULT;

	SPI_PTRS[id]->CTAR[nctar] |= SPI_CTAR_FMSZ(fsize-1);
}

static void SPI_clock_polarity(SPI_id id, CTAR_id nctar, SPI_clock_polarity_t cpol)
{
	SPI_PTRS[id]->CTAR[nctar] |= SPI_CTAR_CPOL(cpol);
}

static void SPI_clock_phase(SPI_id id, CTAR_id nctar, SPI_clock_phase_t cpha)
{
	SPI_PTRS[id]->CTAR[nctar] |= SPI_CTAR_CPHA(cpha);
}

static void SPI_LSB(SPI_id id, CTAR_id nctar,SPI_endiannes bf)
{
	SPI_PTRS[id]->CTAR[nctar] |= (SPI_CTAR_LSBFE(bf));
}

static void SPI_clean_flags(SPI_id id)
{
	SPI_PTRS[id]->SR |= SPI_SR_TCF(1);
	SPI_PTRS[id]->SR |= SPI_SR_EOQF(1);
	SPI_PTRS[id]->SR |= SPI_SR_TFUF(1);
	SPI_PTRS[id]->SR |= SPI_SR_TFFF(1);
	SPI_PTRS[id]->SR |= SPI_SR_RFOF(1);
	SPI_PTRS[id]->SR |= SPI_SR_RFDF(1);
}

static void SPI_IRQ_enable(SPI_id id)
{
	SPI_PTRS[id]->RSER |= SPI_RSER_TCF_RE(1);
	SPI_PTRS[id]->RSER |= SPI_RSER_EOQF_RE(1);
	SPI_PTRS[id]->RSER |= SPI_RSER_TFUF_RE(1);
	SPI_PTRS[id]->RSER |= SPI_RSER_TFFF_RE(1);
	SPI_PTRS[id]->RSER |= SPI_RSER_RFOF_RE(1);
	SPI_PTRS[id]->RSER |= SPI_RSER_RFDF_RE(1);

	SPI_PTRS[id]->RSER |= SPI_RSER_TFFF_DIRS(0); //0=>I, 1=>DMA
	SPI_PTRS[id]->RSER |= SPI_RSER_RFDF_DIRS(0);

	NVIC_EnableIRQ(SPI_IRQS_arr[id]);
}

static void SPI_final_enable(SPI_id id)
{
	SPI_PTRS[id]->MCR = (SPI0->MCR & ~SPI_MCR_MDIS_MASK) | SPI_MCR_MDIS(0);
}

static void SPI_write_frame(SPI_id id, SPI_pushr_cfg_t cfg)
{
    SPI_PTRS[id]->PUSHR = (SPI_PUSHR_CONT(1) | SPI_PUSHR_CTAS(cfg.ctas) | SPI_PUSHR_EOQ(cfg.eoq) | SPI_PUSHR_CTCNT(1) |SPI_PUSHR_PCS(cfg.pcs) | SPI_PUSHR_TXDATA(cfg.data));
}



