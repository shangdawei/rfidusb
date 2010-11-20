#include <msp430f5519.h>
#include "hardware.h"
#include "parallel.h"
#include "SPI.h"
#include "types.h"

/*
 =======================================================================================================================
 =======================================================================================================================
 */
/*
void delay_ms(unsigned int n_ms) {
    unsigned int ii1, ii0;
    for(ii0=n_ms; ii0>0; ii0--) {
        ii1 = 0x07FF;                    // Delay
        do (ii1--);
        while (ii1 != 0);
    }
}
*/
// end of delay_ms

/*
 =======================================================================================================================
 =======================================================================================================================
 */

void CounterSet(void)
{
	TA0CTL |= TACLR;
	TA0CTL &= ~TACLR;			//reset the timerA
	TA0CTL |= TASSEL0 + ID1 + ID0;		//ACLK, div 8, interrupt enable, timer stoped
	
	TA0R = 0x0000;
	TA0CCTL0 |= CCIE;			//compare interrupt enable
}//CounterSet()

 
void init_msp430(void){

	//init_ports();
	init_clock();

} 

void init_ports(void)
{
    // Initialization of ports all unused pins as outputs with low-level
    // set all ports  to low on all pins
    PAOUT   =   0x0000;
    PASEL   =   0x0000;
    PADIR   =   0xFFFF;

    PBOUT   =   0x0000;
    PBSEL   =   0x0000;
    PBDIR   =   0xFFFF;

    PCOUT   =   0x0000;
    PCSEL   =   0x0000;
    PCDIR   =   0xFFFF;

    PDOUT   =   0x0000;
    PDSEL   =   0x0000;
    PDDIR   =   0xFFFF;

    PJDIR   =   0xFFFF;
    PJOUT   =   0x0000;
}
void init_clock(void){
	
	WDTCTL = WDTPW + WDTHOLD; 		// Stop the watchdog

    // XIN and XOUT are configured as GPIOs. To enable XT1 and X2T, the PSEL bits
    // associated with the crystal pin must be set.
    
  	P5SEL |= ((1 << 4) | (1 << 5));   //Low freq XT1 select
	P5SEL |= ((1 << 2) | (1 << 3));	  //High freq XT2 select
	UCSCTL6 &= ~(XT1OFF + XT2OFF);            // Set XT1 & XT2 On
	UCSCTL6 &= ~(XTS); 						//XT1 at low freq XCAP defines capacitance 
	UCSCTL6 |= XCAP_3;                        // Internal load cap XT1 12pF
	UCSCTL6 |= (XT2DRIVE_3);			//Max current, max freq (24-32 MHz)
	UCSCTL6 &= ~(XT2BYPASS);			//Bypass mode OFF
   
   	 do {
    	UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG); 
    	//NOTE: XT1HFOFFG does not exit in msp430f5519
 		//OFIFG oscillator flag interrupt is set and latched at POR
 		//or when any oscillator flag is detected.
    	SFRIFG1 &= ~OFIFG;                      // Clear fault flags
  	}while (SFRIFG1&OFIFG);
  	   
    UCSCTL4 |= SELM__XT2CLK;			//Select XT2 source for MCLK
	UCSCTL5 |= DIVM__4; 				//Divide XT2 freq/2 for MCLK (16 MHz)    
 
   	__delay_cycles(320000); 	
  	 do {
    	UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG); 
    	SFRIFG1 &= ~OFIFG;                     
  	}while (SFRIFG1&OFIFG);

    UCSCTL4 |= SELS__XT2CLK;			//Select XT2 source for SMCLK
  	UCSCTL5 |= DIVS__4; 				//Divide XT2 freq/2 for SMCLK (16 MHz)  	
  	 	
   	__delay_cycles(320000); 	
  	 do {
    	UCSCTL7 &= ~(XT2OFFG + XT1LFOFFG + DCOFFG); 
    	SFRIFG1 &= ~OFIFG;                     
  	}while (SFRIFG1&OFIFG);
  	
  	P2SEL |= BIT2;
  	P2DIR |= BIT2;
}


void delay_us(uint16_t usec) // 5 cycles for calling
{
    // The least we can wait is 3 usec:
    // ~1 usec for call, 1 for first compare and 1 for return
    while(usec > 3)       // 2 cycles for compare
    {                // 2 cycles for jump 
    	_nop();    
        _nop();       // 1 cycles for _nop
        _nop();       // 1 cycles for _nop
        _nop();       // 1 cycles for _nop
        _nop();       // 1 cycles for _nop
        _nop();       // 1 cycles for _nop
        _nop();       // 1 cycles for _nop
        _nop();       // 1 cycles for _nop
        _nop();       // 1 cycles for _nop
/***********************8 MHz***************************
        _nop();       // 1 cycles for _nop
        _nop();       // 1 cycles for _nop
        _nop();       // 1 cycles for _nop
        _nop();       // 1 cycles for _nop
        _nop();       // 1 cycles for _nop
        _nop();       // 1 cycles for _nop
        _nop();       // 1 cycles for _nop
        _nop();       // 1 cycles for _nop
***********************16 MHz**************************
        _nop();       // 1 cycles for _nop
        _nop();       // 1 cycles for _nop
        _nop();       // 1 cycles for _nop
        _nop();       // 1 cycles for _nop
        _nop();       // 1 cycles for _nop
        _nop();       // 1 cycles for _nop
        _nop();       // 1 cycles for _nop
        _nop();       // 1 cycles for _nop
***********************24 MHz**************************
        _nop();       // 1 cycles for _nop
        _nop();       // 1 cycles for _nop
        _nop();       // 1 cycles for _nop
        _nop();       // 1 cycles for _nop
        _nop();       // 1 cycles for _nop
        _nop();       // 1 cycles for _nop
        _nop();       // 1 cycles for _nop
        _nop();       // 1 cycles for _nop
***********************32 MHz**************************/
        usec -= 2;        // 1 cycles for optimized decrement
    }
}                         // 4 cycles for returning


void delay_ms(uint16_t msec)
{
    while(msec-- > 0)
    {
        delay_us(1000);
    }
}

/*
 =======================================================================================================================
 =======================================================================================================================
 */
/*
void OSCsel(unsigned char mode)
{

  	unsigned int ii1;

  	if(mode == 0x00){               //select crystal oscilator
    		BCSCTL1 |= XTS + XT2OFF;                       // ACLK = LFXT1 HF XTAL
                BCSCTL3 |= LFXT1S1;                       // 3 – 16MHz crystal or resonator
		// turn external oscillator on
    		do
    		{
      			IFG1 &= ~OFIFG;                   // Clear OSCFault flag
      			for (ii1 = 0xFF; ii1 > 0; ii1--); // Time delay for flag to set
    		}
    		while ((IFG1 & OFIFG) == OFIFG);    // OSCFault flag still set?
    		BCSCTL2 |= SELM1 + SELM0 + SELS;           //  MCLK = SMCLK = HF LFXT1 (safe)

    		return;
    	}

  	else{                           //select DCO for main clock
    		DCOCTL |= DCO0 + DCO1 + DCO2;
    		BCSCTL1 |= XT2OFF + XTS + RSEL0 + RSEL1 + RSEL2;
   		
		// turn external oscillator on
//    		do
//    		{
//      		IFG1 &= ~OFIFG;                   // Clear OSCFault flag
//      		for (ii1 = 0xFF; ii1 > 0; ii1--); // Time delay for flag to set
//    		}
//    		while ((IFG1 & OFIFG) == OFIFG);    // OSCFault flag still set?
    		BCSCTL2 &= ~(SELM1 + SELM0 + SELS + DCOR);

    		return;
    	}
    	//_BIC_SR(OSCOFF);
}
*/
/*
 =======================================================================================================================
 =======================================================================================================================
 */

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TimerAhandler(void)
{	
	//unsigned char Register;
          unsigned char Register[2];

         //kputchar('V');
	stopCounter;
	
	//Register = IRQStatus;		//IRQ status register address
	//TRF_IRQ_CLEAR();				//PORT2 interrupt flag clear
	//ReadSingle(&Register, 1);	//function call for single address read
					//IRQ status register has to be read
        Register[0] = IRQStatus;	/* IRQ status register address */
        Register[1] = IRQMask;		//Dummy read	
	//ReadSingle(Register, 2);	/* function call for single address read */
        //ReadCont(Register, 1);
        ReadCont(Register, 2);
       // ReadSingle(Register, 1);



        *Register = *Register & 0xF7;	//set the parity flag to 0

	
	//if(Register == 0x00)
        if(*Register == 0x00 || *Register == 0x80) //added code
          	//if(RXTXstate > 1)
                //  	i_reg = 0xFF;
        	//else
			i_reg = 0x00;
	else
		i_reg = 0x01;
	
	__low_power_mode_off_on_exit();
	
}//TimerAhandler
/*
void PortRead(unsigned char *pbuf){
	
	*pbuf=((P4IN & 0xF7)|(P3IN & 0x08));
}*/
void PortWriteSet(uint8_t pbuf){
/*
if (pbuf&0x01) P3OUT |= BIT7; else P3OUT &= ~BIT7;
if (pbuf&0x02) P4OUT |= BIT4; else P4OUT &= ~BIT4;
if (pbuf&0x04) P4OUT |= BIT5; else P4OUT &= ~BIT5;
if (pbuf&0x08) P4OUT |= BIT6; else P4OUT &= ~BIT6;
if (pbuf&0x10) P4OUT |= BIT0; else P4OUT &= ~BIT0;
if (pbuf&0x20) P4OUT |= BIT7; else P4OUT &= ~BIT7;
if (pbuf&0x40) P4OUT |= BIT2; else P4OUT &= ~BIT2;
if (pbuf&0x80) P4OUT |= BIT1; else P4OUT &= ~BIT1;*/
}
void PortWrite(uint8_t *pbuf){
/*
if (*pbuf&0x01) P3OUT |= BIT7; else P3OUT &= ~BIT7; //IO0
if (*pbuf&0x02) P4OUT |= BIT4; else P4OUT &= ~BIT4; //IO1
if (*pbuf&0x04) P4OUT |= BIT5; else P4OUT &= ~BIT5; //IO2
if (*pbuf&0x08) P4OUT |= BIT6; else P4OUT &= ~BIT6; //IO3
if (*pbuf&0x10) P4OUT |= BIT0; else P4OUT &= ~BIT0; //IO4
if (*pbuf&0x20) P4OUT |= BIT7; else P4OUT &= ~BIT7; //IO5
if (*pbuf&0x40) P4OUT |= BIT2; else P4OUT &= ~BIT2; //IO6
if (*pbuf&0x80) P4OUT |= BIT1; else P4OUT &= ~BIT1; //IO7*/
}
void spi_clk_init(void){
  //  uint32_t divider = 0;
    UCB1CTL1 |= UCSWRST;                          // Put state machine in reset
   // UCB1BR0 = LO_UINT32(divider);
   // UCB1BR1 = HI_UINT32(divider);
					 
    TRF_DCLK_SEL();             // P4.3 peripheral select (mux to ACSI_A1)                             						 // P4.3 peripheral select CLK (mux to ACSI_A1)
    
    UCB1CTL0 |= UCCKPH | UCSYNC | UCMSB | UCMST; // 3-pin, 8-bit SPI master, rising edge capture
    UCB1CTL1 = UCSSEL0 | UCSSEL1;                // Select SMCLK
    
    UCB1CTL1 &= ~UCSWRST; 
}

