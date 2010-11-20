#include <msp430f5519.h>   
#include "parallel.h"
#include "SPI.h"
#include "globals.h"
#include "types.h"
#include "hardware.h"

#define DBG 0

unsigned char temp;
unsigned int DUMMYREAD = 0;
unsigned int mask = 0x80;


/*
 =======================================================================================================================
 =======================================================================================================================
 */
void PARset(void)
{
	
	TRF_EN_INIT();
	
    TRFDirOUT;			/* P4 output */
	TRFFunc;
	TRFWrite = 0x00;	/* P4 set to 0 - choose parallel inteface for the TRF796x */
	
	//PortWriteSet(0x00);
	//spi_clk_init();
	TRF_DCLK_OFF();
	TRF_DCLK_INIT();			/* DATA_CLK on P3.3 */



	TRF_IRQ_INIT();
	TRF_IRQ_RISING_EDGE();			/* rising edge interrupt */

	leds_off();
	leds_init();
}						/* PARset */

/*
 =======================================================================================================================
 =======================================================================================================================
 */

void STOPcondition(void)
{
	
	TRFWrite |= 0x80;	/* stop condition */
	//P4OUT |= BIT1; //P4.1 corresponds to IO7 (P4.7 in old file)
	TRF_DCLK_ON();
	TRFWrite = 0x00;
	//PortWriteSet(0x00);
	TRF_DCLK_OFF();
}						/* STOPcondition */

/*
 =======================================================================================================================
 =======================================================================================================================
 */

void STOPcont(void)
{	/* stop condition for continous mode */
	TRFWrite = 0x00;
	//PortWriteSet(0x00);
	TRFDirOUT;
	TRFWrite = 0x80;
	//PortWriteSet(0x80);
	__no_operation();
	TRFWrite = 0x00;
	//PortWriteSet(0x00);
}	/* STOPcond */

/*
 =======================================================================================================================
 =======================================================================================================================
 */

void STARTcondition(void)
{
	TRFWrite = 0x00;
	//PortWriteSet(0x00);
	TRF_DCLK_ON();
	TRFWrite = 0xff;
	//PortWriteSet(0xFF);
	TRF_DCLK_OFF();
}	/* STARTcondition */

///////////////////////////////////////////////////////////////
void SPIStartCondition(void)
{
                //Make the SCLK High
                 P4SEL &= ~ BIT3;
                 P4DIR |= BIT3;
                 P4OUT |= BIT3; //Make SCLK High


                 //Revert back
                 //P4SEL |=  BIT3;

}
///////////////////////////////////////////////////////////////

void SPIStopCondition(void)
{
                //Make the SCLK Low
                 P4SEL &= ~ BIT3;
                 P4OUT &= ~BIT3; //Make SCLK Low
                 P4DIR |= BIT3;
                 P4OUT &= ~BIT3; //Make SCLK Low


                 //Revert back
                 //P4SEL |=  BIT3;

}
///////////////////////////////////////////////////////////////
/*
 =======================================================================================================================
    Function writes only one register or a multiple number ;
    of registers with specified addresses ;
 =======================================================================================================================
 */
void WriteSingle(uint8_t *pbuf, unsigned char lenght)
{
	/*~~~~~~~~~~~~~~*/
	unsigned char	i;
	/*~~~~~~~~~~~~~~*/

         if ((SPIMODE)==0)  //Parallel Mode
         {
	  STARTcondition();
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

	STOPcondition();

         } //end of Parallel mode

      /*********************************************************/

         if (SPIMODE) //SPI Mode

         {

           #ifndef SPI_BITBANG
                       /*********************************/
                       /* Start of Hardware SPI Mode */
                       /*********************************/


                       TRF_SS_LOW(); //Start SPI Mode

                          while(lenght > 0)
                          {
                            *pbuf = (0x1f &*pbuf);	/* register address */
                            for(i = 0; i < 2; i++)
                            {
                               while (!(UCB1IFG & UCTXIFG));            // USCI_B0 TX buffer ready?
                               UCB0TXBUF = *pbuf;                  // Previous data to TX, RX

                               // while (!(UCB1IFG & UCRXIFG));
                                temp=UCB0RXBUF;

                                pbuf++;
                                lenght--;
                            }
                          }	/* while */
                       TRF_SS_HIGH(); //Stop SPI Mode

          #endif


   }  //end of SPI mode

}		/* WriteSingle */






/*
 =======================================================================================================================
    Function writes a specified number of registers from ;
    a specified address upwards ;
 =======================================================================================================================
 */
void WriteCont(unsigned char *pbuf, unsigned char lenght)
{
      /*~~~~~~~~~~~~~~*/
	
        /*~~~~~~~~~~~~~~*/
       if ((SPIMODE)==0) //Parallel Mode
       {	STARTcondition();
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

	  STOPcont();
       } //end of Parallel Mode


      if (SPIMODE)
      {
              #ifndef SPI_BITBANG
                       /*********************************/
                       /* Start of Hardware SPI Mode */
                       /*********************************/
                              TRF_SS_LOW(); //Start SPI Mode
                              *pbuf = (0x20 | *pbuf); /* address, write, continous */
                              *pbuf = (0x3f &*pbuf);	/* register address */
                              while(lenght > 0)
                              {
                                      while (!(UCB1IFG & UCTXIFG));            // USCI_B0 TX buffer ready?
                                      UCB0TXBUF = *pbuf;                  // Previous data to TX, RX

                                      while (!(UCB1IFG & UCRXIFG));
                                      temp=UCB0RXBUF;


                                      pbuf++;
                                      lenght--;
                              }						/* while */

                          TRF_SS_HIGH(); //Stop SPI Mode



              #endif


      } //end of SPI mode

}	/* WriteCont */

/*
 =======================================================================================================================
    Function reads only one register ;
 =======================================================================================================================
 */
void ReadSingle(unsigned char *pbuf, unsigned char lenght)
{
        /*~~~~~~~~~~~~~~*/
	
        /*~~~~~~~~~~~~~~*/

	if ((SPIMODE)==0)
        {

          STARTcondition();
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

	STOPcondition();
        } //end of parallel mode
/**************************************************************/

        if (SPIMODE) //SPI Mode
        {
            #ifndef SPI_BITBANG
                       /*********************************/
                       /* Start of Hardware SPI Mode */
                       /*********************************/
                      TRF_SS_LOW(); //Start SPI Mode



                         while(lenght > 0)
                      {

                            *pbuf = (0x40 | *pbuf); /* address, read, single */
                            *pbuf = (0x5f & *pbuf);	/* register address */

                             while (!(UCB1IFG & UCTXIFG));            // USCI_B0 TX buffer ready?
                             UCB0TXBUF = *pbuf;                  // Previous data to TX, RX

                             //while (!(UCB1IFG & UCRXIFG));
                             temp=UCB0RXBUF;

                             UCB0CTL0 &= ~UCCKPH;

                             /////////////////////////////////SPIStartCondition();//SCLK High
                             /////////////////////////////////P3SEL |=  BIT3;

                             while (!(UCB1IFG & UCTXIFG));            // USCI_B0 TX buffer ready?
                             UCB0TXBUF = 0x00; //Receive initiated by a dummy TX write???

                             while (!(UCB1IFG & UCRXIFG)); //Comment this - Harsha??
                              _NOP();
                              _NOP();
                             *pbuf = UCB0RXBUF;
                             pbuf++;
                            lenght--;

                            UCB0CTL0 |= UCCKPH;

                      }//end of while

                      TRF_SS_HIGH(); //Stop SPI Mode

                         #endif

    } //end of SPI Mode
}		/* ReadSingle */

/*
 =======================================================================================================================
    Function reads specified number of registers from a ;
    specified address upwards. ;
 =======================================================================================================================
 */
void ReadCont(unsigned char *pbuf, unsigned char lenght)
{
  /*~~~~~~~~~~~~~~*/
	unsigned char	j;
      // unsigned char len1 = lenght;

  /*~~~~~~~~~~~~~~*/
      if ((SPIMODE)==0) //Parallel Mode
      {
	STARTcondition();
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

	STOPcont();
      } //end of Parallel Mode


      if (SPIMODE) //
      {
                   #ifndef SPI_BITBANG
                               /*********************************/
                               /* Start of Hardware SPI Mode */
                               /*********************************/
                               // lenght=lenght+2;
                              //  unsigned char len1 = lenght;

                                TRF_SS_LOW(); //Start SPI Mode
                                *pbuf = (0x60 | *pbuf); /* address, read, continous */
                                *pbuf = (0x7f &*pbuf);	/* register address */
                                 while (!(UCB1IFG & UCTXIFG));            // USCI_B0 TX buffer ready?
                                 UCB0TXBUF = *pbuf;                  // Previous data to TX, RX

                                 //while (!(UCB1IFG & UCRXIFG));
                                 temp=UCB0RXBUF;
                                 UCB0CTL0 &= ~UCCKPH;

                                 //added length clause - May 9th 2007
                                 if(*pbuf != 0x6C)//execute only when IRQRead is not called
                                {

                                 if (lenght != 0x1F)

                                 {

                                 for (j=0;j<2;j++)
                                 {
                                    while (!(UCB1IFG & UCTXIFG));
                                     UCB0TXBUF = 0x00; //Receive initiated by a dummy TX write


                                      while (!(UCB1IFG & UCRXIFG));
                                      _NOP();
                                      _NOP();
                                      temp = UCB0RXBUF;
                                  }
                                    }
                                  }



                                 while(lenght > 0)
                                 {

                                     while (!(UCB1IFG & UCTXIFG));
                                     UCB0TXBUF = 0x00; //Receive initiated by a dummy TX write



                                      while (!(UCB1IFG & UCRXIFG));
                                      _NOP();
                                      _NOP();
                                     *pbuf = UCB0RXBUF;
                                      pbuf++;
                                     lenght--;

                                  }
                              UCB0CTL0 |= UCCKPH;
                              TRF_SS_HIGH(); //Stop SPI Mode

                  #endif

      } //end of SPI mode

}	/* ReadCont */

/*
 =======================================================================================================================
    Function DirectCommand transmits a command to the reader chip
 =======================================================================================================================
 */
void DirectCommand(unsigned char *pbuf)
{
  /*~~~~~~~~~~~~~~*/
  
  /*~~~~~~~~~~~~~~*/
      if ((SPIMODE)==0)	//Parallel Mode
      { STARTcondition();
	*pbuf = (0x80 | *pbuf); /* command */
	*pbuf = (0x9f &*pbuf);	/* command code */
	TRFWrite = *pbuf;		/* send command */
	//PortWrite(pbuf);
	TRF_DCLK_ON();
	TRF_DCLK_OFF();
	STOPcondition();
      }

      if (SPIMODE)    //SPI Mode
      {
                #ifndef SPI_BITBANG
                       /*********************************/
                        /* Start of Hardware SPI Mode */
                        /*********************************/
                       TRF_SS_LOW(); //Start SPI Mode
                       *pbuf = (0x80 | *pbuf); /* command */
                       *pbuf = (0x9f &*pbuf);	/* command code */
                        while (!(UCB1IFG & UCTXIFG));            // USCI_B0 TX buffer ready?
                        UCB0TXBUF = *pbuf;                  // Previous data to TX, RX

                      //  while (!(UCB1IFG & UCRXIFG));
                        temp=UCB0RXBUF;


                        SPIStartCondition(); //SCLK High
                //        SPIStopCondition();  // SCLK Low
                //  P3SEL |=  BIT3;      //Revert Back

                        TRF_SS_HIGH(); //Stop SPI Mode

                        P3SEL |=  BIT3;      //Revert Back

               #endif

      }

}	/* DirectCommand */

/*
 =======================================================================================================================
    Function used for direct writing to reader chip ;
 =======================================================================================================================
 */
void RAWwrite(unsigned char *pbuf, unsigned char lenght)
{
  /*~~~~~~~~~~~~~~*/
   
  /*~~~~~~~~~~~~~~*/
  if ((SPIMODE)==0) //Parallel Mode
  {
        STARTcondition();
	while(lenght > 0)
	{
		TRFWrite = *pbuf;	/* send command */
		//PortWrite(pbuf);
		TRF_DCLK_ON();
		TRF_DCLK_OFF();
		pbuf++;
		lenght--;
	}						/* while */

	STOPcont();
  }//end Parallel Mode

/**********************************************************************/
  if (SPIMODE)
  {
         #ifndef SPI_BITBANG
                       /*********************************/
                        /* Start of Hardware SPI Mode */
                        /*********************************/
                      TRF_SS_LOW(); //Start SPI Mode


                       while(lenght > 0)
                        {

                                while (!(UCB1IFG & UCTXIFG));            // USCI_B0 TX buffer ready?
                                UCB0TXBUF = *pbuf;                  // Previous data to TX, RX


                               // while (!(UCB1IFG & UCRXIFG));
                               temp=UCB0RXBUF;

                                pbuf++;
                               lenght--;

                        }						/* while */
                        // SPIStartCondition(); //SCLK High

                         TRF_SS_HIGH(); //Stop SPI Mode
                      //    SPIStartCondition(); //SCLK High
                      //   P3SEL |=  BIT3;      //Revert Back

                          // end added code


        #endif
  }//end of SPI Mode

}	/* RAWwrite */

/*
 =======================================================================================================================
    Direct mode (no stop condition) ;
 =======================================================================================================================
 */
void DirectMode(void)
{
    if ((SPIMODE)==0)//Parallel Mode
      {
        //OOKdirOUT;
	STARTcondition();
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
      }//end of Parallel mode

      if (SPIMODE) //SPI Mode
      {
          #ifndef SPI_BITBANG
                         //OOKdirOUT;
                         TRF_SS_LOW(); //Start SPI Mode
                         while (!(UCB1IFG & UCTXIFG));            // USCI_B0 TX buffer ready?
                         UCB0TXBUF = ChipStateControl;                  // Previous data to TX, RX

                         while (!(UCB1IFG & UCRXIFG));
                          temp=UCB0RXBUF;


                         while (!(UCB1IFG & UCTXIFG));            // USCI_B0 TX buffer ready?
                         UCB0TXBUF = 0x61;                  /* write a 1 to BIT6 in register * 0x00;  * */
                         TRFDirIN;			/* put the PORT1 to tristate */

        #endif



      }  //end of SPI Mode

}						/* DirectMode */


/*
 =======================================================================================================================
    Send a specified number of bytes from buffer to host ;
 =======================================================================================================================
 */
void Response(unsigned char *pbuf, unsigned char lenght)
{
	/*
	 * char msg[40];
	 */
	while(lenght > 0)
	{
		/*
		 * sprintf(msg, "[%x]", *pbuf++);
		 * *send_cstring(msg);
		 */
		kputchar('[');
		Put_byte(*pbuf);
		kputchar(']');
		pbuf++;
		lenght--;
	}

	put_crlf();
}	/* Response */

/*
 =======================================================================================================================
 =======================================================================================================================
 */

void InitialSettings(void)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~*/
	unsigned char	command[2];
	/*~~~~~~~~~~~~~~~~~~~~~~~*/

	command[0] = ModulatorControl;
	command[1] = 0x21;   //6.78 MHz
    //command[1] = 0x31;  //13.56 MHz

	WriteSingle(command, 2);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */

void ReInitialize15693Settings(void)
{
	/*~~~~~~~~~~~~~~~~~~~~~~~*/
	unsigned char	command2[2];
	/*~~~~~~~~~~~~~~~~~~~~~~~*/

	command2[0] = RXNoResponseWaitTime;
	command2[1] = 0x14;//20
        WriteSingle(command2, 2);

        command2[0] = RXWaitTime;
	command2[1] = 0x20; //32
	WriteSingle(command2, 2);
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
void InterruptHandlerReader(unsigned char *Register)
{
	/*~~~~~~~~~~~~~~~~*/
	/*
	 * char phello[20];
	 * //for testing
	 */
	unsigned char	len;
	/*~~~~~~~~~~~~~~~~*/

	/*
	 * Register = Register & 0xF7;
	 * //set the parity flag to 0 ;
	 * parity is not used in 15693 and Tag-It
	 */
#if DBG
	Put_byte(*Register);
#endif
	if(*Register == 0xA0)
	{					/* TX active and only 3 bytes left in FIFO */
		i_reg = 0x00;
#if DBG
		kputchar('.');	/* FIFO filling is done in the transmit function */
#endif
	}

	else if(*Register == BIT7)
	{					/* TX complete */
		i_reg = 0x00;
		*Register = Reset;			/* reset the FIFO after TX */
		DirectCommand(Register);
#if DBG
		kputchar('T');
#endif
	}

	else if((*Register & BIT1) == BIT1)
	{								/* collision error */
		i_reg = 0x02;				/* RX complete */

//		*Register = StopDecoders;	/* reset the FIFO after TX */
//		DirectCommand(Register);

		CollPoss = CollisionPosition;
		ReadSingle(&CollPoss, 1);
		
//		*Register = 0x0c;
//		ReadSingle(Register, 1);
		/*
		 * CollPoss &= 0xF7;
		 */
		len = CollPoss - 0x20;		/* number of valid bytes if FIFO */

		if(!POLLING)
		{
			kputchar('{');
			Put_byte(CollPoss);
			kputchar('}');
		}

		if((len & 0x0f) != 0x00) len = len + 0x10;	/* add 1 byte if broken byte recieved */
		len = len >> 4;

		if(len != 0x00)
		{
			buf[RXTXstate] = FIFO;					/* write the recieved bytes to the correct place of the
													 * buffer;
													 * */
			ReadCont(&buf[RXTXstate], len);
			RXTXstate = RXTXstate + len;
		}						/* if */
		*Register = StopDecoders;	/* reset the FIFO after TX */
		DirectCommand(Register);
		
		*Register = Reset;
		DirectCommand(Register);

		*Register = IRQStatus;	/* IRQ status register address */
                 *(Register + 1) = IRQMask;
		//ReadSingle(Register, 2);	/* function call for single address read */
                // ReadCont(Register, 2);
                    if (SPIMODE)
                       ReadCont(Register, 2);
                    else
                        ReadSingle(Register, 1);	

		TRF_IRQ_CLEAR();
	}
	else if(*Register == BIT6)
	{	/* RX flag means that EOF has been recieved */
		/* and the number of unread bytes is in FIFOstatus regiter */
		if(RXErrorFlag == 0x02)
		{
			i_reg = 0x02;
			return;
		}

		*Register = FIFOStatus;
		ReadSingle(Register, 1);					/* determine the number of bytes left in FIFO */
//                ReadCont(Register, 1);	
		*Register = (0x0F &*Register) + 0x01;
		buf[RXTXstate] = FIFO;			/* write the recieved bytes to the correct place of the*/
													
												
		ReadCont(&buf[RXTXstate], *Register);
		RXTXstate = RXTXstate +*Register;

		*Register = TXLenghtByte2;					/* determine if there are broken bytes */
	 //       ReadSingle(Register, 1);					/* determine the number of bits */
                  ReadCont(Register, 1);

		if((*Register & BIT0) == BIT0)
		{
			*Register = (*Register >> 1) & 0x07;	/* mask the first 5 bits */
			*Register = 8 -*Register;
			buf[RXTXstate - 1] &= 0xFF << *Register;
		}								/* if */

#if DBG
		kputchar('E');
#endif
		*Register = Reset;				/* reset the FIFO after last byte has been read out */
		DirectCommand(Register);

		i_reg = 0xFF;					/* signal to the recieve funnction that this are the last bytes */
	}
	else if(*Register == 0x60)
	{									/* RX active and 9 bytes allready in FIFO */
		i_reg = 0x01;
		buf[RXTXstate] = FIFO;
		ReadCont(&buf[RXTXstate], 9);	/* read 9 bytes from FIFO */
		RXTXstate = RXTXstate + 9;
#if DBG
		kputchar('F');
#endif
		if(TRF_IRQ_PORT & TRF_IRQ_PIN)
		{
			*Register = IRQStatus;		/* IRQ status register address */
                        *(Register + 1) = IRQMask;
			//ReadSingle(Register, 2);	/* function call for single address read */
                        //ReadCont(Register, 2);
                           if (SPIMODE)
                              ReadCont(Register, 2);
                          else
                                ReadSingle(Register, 1);	
			TRF_IRQ_CLEAR();

			if(*Register == 0x40)
			{	/* end of recieve */
				*Register = FIFOStatus;
				ReadSingle(Register, 1);					/* determine the number of bytes left in FIFO */
                                //ReadCont(Register, 1);
				*Register = 0x0F & (*Register + 0x01);
				buf[RXTXstate] = FIFO;						/* write the recieved bytes to the correct place of the*/
															
															
				ReadCont(&buf[RXTXstate], *Register);
				RXTXstate = RXTXstate +*Register;

				*Register = TXLenghtByte2;					/* determine if there are broken bytes */
				ReadSingle(Register, 1);					/* determine the number of bits */
                                //ReadCont(Register, 1);

				if((*Register & BIT0) == BIT0)
				{
					*Register = (*Register >> 1) & 0x07;	/* mask the first 5 bits */
					*Register = 8 -*Register;
					buf[RXTXstate - 1] &= 0xFF << *Register;
				}						/* if */

#if DBG
				kputchar('E');
#endif
				i_reg = 0xFF;			/* signal to the recieve funnction that this are the last bytes */
				*Register = Reset;		/* reset the FIFO after last byte has been read out */
				DirectCommand(Register);
			}
			else if(*Register == 0x50)
			{							/* end of recieve and error */
				i_reg = 0x02;
#if DBG
				kputchar('x');
#endif
			}
		}
		else
		{
			Register[0] = IRQStatus;
                        Register[1] = IRQMask;
			//ReadSingle(Register, 2);	/* function call for single address read */
                        //ReadCont(Register, 2);
                           if (SPIMODE)
                                ReadCont(Register, 2);
                           else
                                 ReadSingle(Register, 1);	
			if(Register[0] == 0x00) i_reg = 0xFF;
		}
	}
	else if((*Register & BIT4) == BIT4)
	{						/* CRC error */
		if((*Register & BIT5) == BIT5)
		{
			i_reg = 0x01;	/* RX active */
			RXErrorFlag = 0x02;
		}
		else
			i_reg = 0x02;	/* end of RX */
	}
	else if((*Register & BIT2) == BIT2)
	{						/* byte framing error */
		if((*Register & BIT5) == BIT5)
		{
			i_reg = 0x01;	/* RX active */
			RXErrorFlag = 0x02;
		}
		else
			i_reg = 0x02;	/* end of RX */
	}
	else if((*Register == BIT0))
	{						/* No response interrupt */
		i_reg = 0x00;
#if DBG
		kputchar('N');
#endif
	}
	else
	{						/* Interrupt register not properly set */
		if(!POLLING)
		{
			/*
			 * sprintf(phello, "Interrupt error. %x\n\r", *Register);
			 * *send_cstring(phello);
			 */
			send_cstring("Interrupt error");
			Put_byte(*Register);
		}

		i_reg = 0x02;

		*Register = StopDecoders;	/* reset the FIFO after TX */
		DirectCommand(Register);

		*Register = Reset;
		DirectCommand(Register);

		*Register = IRQStatus;		/* IRQ status register address */
                *(Register + 1) = IRQMask;
		//ReadSingle(Register, 2);	/* function call for single address read */
                //ReadCont(Register, 2);
                   if (SPIMODE)
                      ReadCont(Register, 2);
                   else
                       ReadSingle(Register, 1);	
		TRF_IRQ_CLEAR();
	}
}	/* InterruptHandlerReader */

/*
 =======================================================================================================================
 =======================================================================================================================
 */

#pragma vector = PORT1_VECTOR
__interrupt void Port_B (void)		/* interrupt handler */
{
	unsigned char Register[4];
	if ((P1IFG & (1 << 5)) && (P1IE & (1 << 5))){
		stopCounter;					/* stop timer mode */
	
		do
		{
			TRF_IRQ_CLEAR();						/* PORT2 interrupt flag clear */
			Register[0] = IRQStatus;	/* IRQ status register address */
	        	Register[1] = IRQMask;		//Dummy read	
	            //    ReadSingle(Register, 2);	/* function call for single address read */
	                if (SPIMODE)
	                 ReadCont(Register, 2);
	                else
	                  ReadSingle(Register, 1);	/* function call for single address read */
	
			/* IRQ status register has to be read */
	
			if(*Register == 0xA0)
			{	/* TX active and only 3 bytes left in FIFO */
				goto FINISH;
			}
	
	
			InterruptHandlerReader(&Register[0]);
		} while((TRF_IRQ_PORT & TRF_IRQ_PIN) == TRF_IRQ_PIN);
	FINISH:
		__low_power_mode_off_on_exit();
	}
}


