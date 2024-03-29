/****************************************************************
*
* Copyright (C) 2006-2007 Texas Instruments, Inc.
*ShreHarsha Rao - RFID Systems
*----------------------------------------------------------------
* All software and related documentation is provided "AS IS" and
* without warranty or support of any kind and Texas Instruments
* expressly disclaims all other warranties, express or implied,
* including, but not limited to, the implied warranties of
* merchantability and fitness for a particular purpose. Under no
* circumstances shall Texas Instruments be liable for any
* incidental, special or consequential damages that result from
* the use or inability to use the software or related
* documentation, even if Texas Instruments has been advised of
* the liability.
*
* Unless otherwise stated, software written and copyrighted by
* Texas Instruments is distributed as "freeware". You may use
* and modify this software without any charge or restriction.
* You may distribute to others, as long as the original author
* is acknowledged.
*
****************************************************************/




//--------------------------------------
//Program with hardware USART and
//paralel communication interface
//with TIR reader chip.
//

//PORT 4.0-4.7 - (IO0-IO7) for parallel interface with reader chip
//PORT3.4 - PORT3.5 - USCI_A0 ---> USB/UART control signals
//PORT2.1 - IRQ
//PORT3.3 - DATA_CLK
//PORT1.7 - PORT1.3 - signaling LEDs
//--------------------------------------

#define DBG		0


#include <msp430f5519.h>     	
#include <stdlib.h>		//Processor specific definitions
#include <stdio.h>
#include "hardware.h"
#include "parallel.h"
#include "anticollision.h"
#include "globals.h"
#include "tiris.h"
#include "14443.h"
#include "host.h"
#include "epc.h"
#include "automatic.h"
#include "types.h"
//#include "mcu.h"


// =======================================================================

char rxdata;			//RS232 RX data byte
unsigned char buf[BUF_LENGTH];
unsigned char buf_tmp[TMP_BUF_LENGTH];
signed char RXTXstate;	//used for transmit recieve byte count
unsigned char flags;	//stores the mask value (used in anticollision)
unsigned char AFI = 0;
unsigned char RXErrorFlag;
unsigned char RXflag;		//indicates that data is in buffer
unsigned char i_reg;	//interrupt register
unsigned char counter;
unsigned char CollPoss;
unsigned char GUI;
// =========================================================================
// Main function with init and an endless loop
//
//


  void main(void) {
//	uint8_t command[2];
    // initialize peripherals
    WDTCTL = WDTPW + WDTHOLD;             // Stop WDT

    //UARTset();
	init_msp430();

    TRF_EN_INIT();
    TRF_DISABLE();
    delay_ms(1);
    TRF_ENABLE();
    delay_ms(1);

    //Add logic for SPI/parallel selection.
    if (SPIMODE)
    { //Set Port Functions for Serial Mode
    TRF_EN_INIT();
	TRF_IRQ_INIT();
	TRF_IRQ_RISING_EDGE();			/* rising edge interrupt */

	leds_off();
	leds_init();
    }
    else
      PARset();     //Set Port Functions for Parallel Mode

    InitialSettings();                  // Set MCU Clock Frequency to 13.56 MHz and OOK Modulation

    EnableInterrupts;                    // General enable interrupts

    delay_ms(10);
    LEDpowerON;


/*
	command[0] = ISOControl;	
	command[1] = 0x02;
	WriteSingle(command, 2);
	ReadSingle(command, 1);
	command[0] = TXtimerEPChigh;	
	ReadSingle(command, 1);
	command[0] = Reset;
	DirectCommand(command);
	command[0] = RunDecoders;
	DirectCommand(command);
	command[0] = ChectInternalRF;
	DirectCommand(command);
	
	command[0] = RSSILevels;	// read RSSI levels 
	ReadSingle(command, 1);
	
	command[0] = RegulatorControl;	
	ReadSingle(command, 1);
    //OOKdirIN;			//set OOK port tristate
*/
    ENABLE = 1;

    POLLING = 1;
    GUI = 0;
    //FindTags(0x00);
   while (1)
    {
    	if (GUI) 
    	{
    		//buf[4] = 0xff;
    		//HostCommands ();
    		POLLING = 0;
    	}
    	else 	FindTags(0x00);
    }
}


// === end of main =====================================================



