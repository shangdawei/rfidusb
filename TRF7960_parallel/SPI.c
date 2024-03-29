#include "SPI.h"
#include "globals.h"
#include "hardware.h"
#define DBG 0

/*
 =======================================================================================================================
 =======================================================================================================================
 */

/*void USARTset(void)
{

}*/


// =======================================================================================================================
//    Function writes only one register or a multiple number ;
//    of registers with specified addresses ;
// =======================================================================================================================

void SPIWriteSingle(unsigned char *pbuf, unsigned char lenght)
{
	/*~~~~~~~~~~~~~~*/
	unsigned char	i;
	/*~~~~~~~~~~~~~~*/

	//STARTcondition();
	while(lenght > 0)
	{
		*pbuf = (0x1f &*pbuf);	/* register address */

		/* address, write, single */
		for(i = 0; i < 2; i++)
		{
			TRFWrite = *pbuf;	/* send command and data */
			//PortWrite(pbuf);
			TRF_DCLK_ON();
			TRF_DCLK_OFF();
			pbuf++;
			lenght--;
		}
	}	/* while */

	//STOPcondition();
}		/* WriteSingle */

/*
 =======================================================================================================================
    Function writes a specified number of registers from ;
    a specified address upwards ;
 =======================================================================================================================
 */
void SPIWriteCont(unsigned char *pbuf, unsigned char lenght)
{
	//STARTcondition();
	*pbuf = (0x20 | *pbuf); /* address, write, continous */
	*pbuf = (0x3f &*pbuf);	/* register address */
	while(lenght > 0)
	{
		TRFWrite = *pbuf;	/* send command */
		//PortWrite(pbuf);
		TRF_DCLK_ON();
		TRF_DCLK_OFF();
		pbuf++;
		lenght--;
	}						/* while */

	//STOPcont();
}	/* WriteCont */

/*
 =======================================================================================================================
    Function reads only one register ;
 =======================================================================================================================
 */
void SPIReadSingle(unsigned char *pbuf, unsigned char lenght)
{
	//STARTcondition();
	while(lenght > 0)
	{
		*pbuf = (0x40 | *pbuf); /* address, read, single */
		*pbuf = (0x5f &*pbuf);	/* register address */

		TRFWrite = *pbuf;		/* send command */
		//PortWrite(pbuf);
		TRF_DCLK_ON();
		TRF_DCLK_OFF();

		TRFDirIN;				/* read register */
		TRF_DCLK_ON();
		__no_operation();
		*pbuf = PortRead;
		TRF_DCLK_OFF();

		TRFWrite = 0x00;
		//PortWriteSet(0x00);
		TRFDirOUT;

		pbuf++;
		lenght--;
	}	/* while */

	//STOPcondition();
}		/* ReadSingle */

/*
 =======================================================================================================================
    Function reads specified number of registers from a ;
    specified address upwards. ;
 =======================================================================================================================
 */
void SPIReadCont(unsigned char *pbuf, unsigned char lenght)
{
	//STARTcondition();
	*pbuf = (0x60 | *pbuf); /* address, read, continous */
	*pbuf = (0x7f &*pbuf);	/* register address */
	TRFWrite = *pbuf;		/* send command */
	//PortWrite(pbuf);
	TRF_DCLK_ON();
	TRF_DCLK_OFF();
	TRFDirIN;				/* read register */

	/*
	 * TRFWrite = 0x00;
	 */
	while(lenght > 0)
	{
		TRF_DCLK_ON();

		/*
		 * TRFDirIN;
		 */
		__no_operation();
		*pbuf = PortRead;

		/*
		 * TRFDirOUT;
		 */
		TRF_DCLK_OFF();
		pbuf++;
		lenght--;
	}						/* while */

	//STOPcont();
}	/* ReadCont */

/*
 =======================================================================================================================
    Function DirectCommand transmits a command to the reader chip
 =======================================================================================================================
 */
void SPIDirectCommand(unsigned char *pbuf)
{
	//STARTcondition();
	*pbuf = (0x80 | *pbuf); /* command */
	*pbuf = (0x9f &*pbuf);	/* command code */
	TRFWrite = *pbuf;		/* send command */
	//PortWrite(pbuf);
	TRF_DCLK_ON();
	TRF_DCLK_OFF();
	//STOPcondition();
}	/* DirectCommand */

/*
 =======================================================================================================================
    Function used for direct writing to reader chip ;
 =======================================================================================================================
 */
void SPIRAWwrite(unsigned char *pbuf, unsigned char lenght)
{
	//STARTcondition();
	while(lenght > 0)
	{
		TRFWrite = *pbuf;	/* send command */
		//PortWrite(pbuf);
		TRF_DCLK_ON();
		TRF_DCLK_OFF();
		pbuf++;
		lenght--;
	}						/* while */

	//STOPcont();
}	/* RAWwrite */

/*
 =======================================================================================================================
    Direct mode (no stop condition) ;
 =======================================================================================================================
 */
void SPIDirectMode(void)
{
	//OOKdirOUT;
	//STARTcondition();
	TRFWrite = ChipStateControl;
	//PortWriteSet(ChipStateControl);
	TRF_DCLK_ON();
	TRF_DCLK_OFF();
	TRFWrite = 0x61;	
	/* write a 1 to BIT6 in register
	 * 0x00;
	 * */
	//PortWriteSet(0x61);
	TRF_DCLK_ON();
	TRF_DCLK_OFF();

	TRFDirIN;			/* put the PORT1 to tristate */
}						/* DirectMode */

/*
 =======================================================================================================================
    Serial Mode - Port Setup ;
 =======================================================================================================================
 */
void SERset(void)
{
	TRF_EN_INIT();

	
	TRF_IRQ_INIT();
	TRF_IRQ_RISING_EDGE();			/* rising edge interrupt */

	leds_off();
	leds_init();
}						/* PARset */



// === Modifications====================================================
// 11/14/06 Harsha - Created new file
// =====================================================================

