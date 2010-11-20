//----------------------------------------------//
//Here are the processor specific functions.	//
//						//
//----------------------------------------------//
#include <msp430f5519.h>     		
#include <stdio.h>
#include "globals.h"
#include "types.h"


/*#ifndef HIGHSPEED
#define HIGHSPEED
#endif*/

//========MCU constants===================================

#define TRFWrite 	P4OUT		//port4 is connected to the
void PortWrite(uint8_t *pbuf);
void PortWriteSet(uint8_t pbuf);
#define PortRead  	P4IN		//TRF796x IO port.
//#define PortRead	((P4IN & 0xF7)|(P3IN & BIT7))

#define TRFDirIN	P4DIR = 0x00
//#define TRFDirIN	P4DIR &= ~(0xF7); P3DIR &= ~BIT7;

#define TRFDirOUT	P4DIR = 0xFF
//#define TRFDirOUT	P4DIR |= (0xF7);\
					P3DIR |= BIT7

#define TRFSerial	P4DIR = 0x9F  //Note sure !!!! - Harsha
//#define TRFSerial	P4DIR |= (0x73); P3DIR |= BIT7;

#define TRFFunc		P4SEL = 0x00
//#define TRFFunc		P4SEL &= ~(0xF7); P3SEL &= ~BIT7;



#define TRF_EN_INIT()			P5DIR |= BIT6
#define	TRF_ENABLE()			P5OUT |= BIT6		//EN pin on the TRF796x
#define TRF_DISABLE()			P5OUT &= ~BIT6


//PIN operations---------------------------------------------
#define TRF_DCLK_INIT()		 	P5DIR |= BIT7		//DATA_CLK on P5.7
#define TRF_DCLK_SEL()		 	P5SEL |= BIT7
#define TRF_DCLK_ON()			P5OUT |= BIT7
#define TRF_DCLK_OFF()			P5OUT &= ~BIT7

#define TRF_SIMO_INIT()         P4DIR |= BIT1          //SIMO on P4.1
#define TRF_SIMO_ON()           P4OUT |= BIT1
#define TRF_SIMO_OFF()          P4OUT &= ~BIT1
#define TRF_SOMI_INIT()         P4DIR &= ~BIT2          //SIMO on P4.1
#define TRF_SOMI_IN()		    P4IN & BIT2		


//Interrupt IRQ--------------------------------
#define TRF_IRQ_INIT()				P1DIR &= ~BIT5
#define TRF_IRQ_PIN					BIT5
#define TRF_IRQ_PORT				P1IN
#define TRF_IRQ_ON()				P1IE |= BIT5		//IRQ on P1.5
#define TRF_IRQ_OFF()				P1IE &= ~BIT5		//IRQ off P1.5
#define TRF_IRQ_RISING_EDGE()		P1IES &= ~BIT5		//rising edge interrupt
#define TRF_IRQ_CLEAR()				P1IFG = 0x00
#define TRF_IRQ_FLAG				P1IFG

#define leds_init()	P8DIR |= (1 << 0)|(1 << 1)|(1 << 2);
#define leds_off()	P8OUT &= ~((1 << 0)|(1 << 1)|(1 << 2));
#define LEDallON	P8OUT |= (1 << 0)|(1 << 1)|(1 << 2);
#define LEDpowerON	P8OUT |= BIT0;
#define LEDpowerOFF	P8OUT &= ~BIT0;
#define LEDtypeAON	P8OUT |= BIT1;
#define LEDtypeAOFF	P8OUT &= ~BIT1;
#define LEDtypeBON	P8OUT |= BIT1;
#define LEDtypeBOFF	P8OUT &= ~BIT1;
#define LED15693ON	P8OUT |= BIT1;
#define LED15693OFF	P8OUT &= ~BIT1;
#define LEDtagitON	P8OUT |= BIT2;
#define LEDtagitOFF	P8OUT &= ~BIT2;
#define LEDopenON  	P8OUT |= BIT2;
#define LEDopenOFF 	P8OUT &= ~BIT2;


//#define SPIMODE             	1           //This is set to Vcc for SPI mode regardless of the jumper at GND or VCC)
//#define SPIMODE               P2IN & BIT3   //This is set to Vcc for SPI mode and GND for Parallel Mode using a separate jumper
#define SPIMODE             	0 		//Parallel mode
#define TRF_SS_INIT()		  	P4DIR |= BIT0
#define TRF_SS_HIGH()   	    P4OUT |= BIT0
#define TRF_SS_LOW()	      	P4OUT &= ~BIT0

/********************
#define OOKdirIN	P2DIR &= ~BIT2
#define OOKdirOUT	P2DIR |= BIT2
#define OOKoff		P2OUT &= ~BIT2
#define OOKon		P2OUT |= BIT2
*********************/

//Counter-timer constants------------------------------------
#define countValue	TA0CCR0			//counter register
//#define startCounter	TA0CTL |= MC0 + MC1	//start counter in up/down mode
#define startCounter	TA0CTL |=  MC1	//start counter in up mode

#define stopCounter	TA0CTL &= ~(MC0 + MC1)	//stops the counter

#ifndef HIGHSPEED
  #define count1ms	847
#else
  #define count1ms	1694
#endif

//=======TRF definitions=========================
//Reader addresses
#define ChipStateControl	0x00
#define ISOControl		0x01
#define ISO14443Boptions	0x02
#define ISO14443Aoptions	0x03
#define TXtimerEPChigh		0x04
#define TXtimerEPClow		0x05
#define TXPulseLenghtControl	0x06
#define RXNoResponseWaitTime	0x07
#define RXWaitTime		0x08
#define ModulatorControl	0x09
#define RXSpecialSettings	0x0A
#define RegulatorControl	0x0B
#define IRQStatus		0x0C
#define IRQMask			0x0D
#define	CollisionPosition	0x0E
#define RSSILevels		0x0F
#define RAMStartAddress		0x10	//RAM is 7 bytes long (0x10 - 0x16)
#define NFCID			0x17
#define NFCTargetLevel		0x18
#define NFCTargetProtocol	0x19
#define TestSetting1		0x1A
#define TestSetting2		0x1B
#define FIFOStatus		0x1C
#define TXLenghtByte1		0x1D
#define TXLenghtByte2		0x1E
#define FIFO			0x1F

//Reader commands-------------------------------------------
#define Idle			0x00
#define SoftInit		0x03
#define InitialRFCollision	0x04
#define ResponseRFCollisionN	0x05
#define ResponseRFCollision0	0x06
#define	Reset			0x0F
#define TransmitNoCRC		0x10
#define TransmitCRC		0x11
#define DelayTransmitNoCRC	0x12
#define DelayTransmitCRC	0x13
#define TransmitNextSlot	0x14
#define CloseSlotSequence	0x15
#define StopDecoders		0x16
#define RunDecoders		0x17
#define ChectInternalRF		0x18
#define CheckExternalRF		0x19
#define AdjustGain		0x1A
//==========================================================

void init_msp430(void);
void init_clock(void);
void init_ports(void);
void delay_us(uint16_t usec);
void delay_ms(uint16_t msec);
void CounterSet(void);
void OSCsel(unsigned char mode);
void spi_clk_init(void);

//#pragma vector=TIMERA0_VECTOR
//__interrupt void TimerAhandler(void);
