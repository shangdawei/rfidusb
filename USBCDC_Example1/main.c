// (c)2009 by Texas Instruments Incorporated, All Rights Reserved.
/*----------------------------------------------------------------------------+
|                                                                             |
|                              Texas Instruments                              |
|                                                                             |
|                          MSP430 USB-Example (CDC/HID Driver)                |
|                                                                             |
+-----------------------------------------------------------------------------+
|  Source: main.c, v1.19 2009/19/11                                           |
------------------------------------------------------------------------------+   
 LED Control Demo:

 This USB demo example is to be used with a PC application (e.g. HyperTerminal)
 This demo application is used to control the operation of the LED at P1.0

 Typing the following pharses in the HyperTerminal Window does the following
 1. "LED ON" Turns on the LED and returns "LED is ON" phrase to PC
 2. "LED OFF" Turns off the LED and returns "LED is OFF" back to HyperTerminal
 3. "LED TOGGLE - SLOW" Turns on the timer used to toggle LED with a large 
     period and returns "LED is toggling slowly" phrase back to HyperTerminal
 4. "LED TOGGLE - FAST" Turns on the timer used to toggle LED with a smaller
     period and returns "LED is toggling fast" phrase back to HyperTerminal

+----------------------------------------------------------------------------+
 Please refer to "USB CDC API Demo Examples Guide.pdf" for more details
//----------------------------------------------------------------------------*/

#include "Common\device.h"
#include "Common\types.h"          // Basic Type declarations
#include ".\USB_Common\descriptors.h"
#include "USB_Common\usb.h"        // USB-specific functions
#include "main.h"
#include "Common\hal_UCS.h"
#include "Common\hal_pmm.h"
    #include "USB_CDC_API\UsbCdc.h"
/*#ifdef _CDC_
    #include "USB_CDC_API\UsbCdc.h"
#endif
#ifdef _HID_
    #include "USB_HID_API\UsbHid.h"
#endif
*/
#include <intrinsics.h>
#include <string.h>
#include "USBCDC_constructs.h"
#include "main.h"

volatile BYTE bDataReceived_event = FALSE; // Indicates data has been received without an open rcv operation
volatile BYTE bDataReceiveCompleted_event = FALSE;  // data receive completed event
volatile BYTE bDataSendCompleted_event = FALSE;     // data send completed event
                  
#define MAX_STR_LENGTH 64
char wholeString[MAX_STR_LENGTH] = "";     // The entire input string from the last 'return'

unsigned int SlowToggle_Period = 20000-1;
unsigned int FastToggle_Period = 1000-1;

/*----------------------------------------------------------------------------+
| Main Routine                                                                |
+----------------------------------------------------------------------------*/
VOID main(VOID)
{
    WDTCTL = WDTPW + WDTHOLD;	    // Stop watchdog timer
    Init_StartUp();                 //initialize device
    Init_TimerA1();
    
    USB_init();
    
    // Enable various USB event handling routines
    USB_setEnabledEvents(kUSB_VbusOnEvent+kUSB_VbusOffEvent+kUSB_receiveCompletedEvent
                          +kUSB_dataReceivedEvent+kUSB_UsbSuspendEvent+kUSB_UsbResumeEvent+kUSB_UsbResetEvent);
    
    // See if we're already attached physically to USB, and if so, connect to it
    // Normally applications don't invoke the event handlers, but this is an exception.  
    if (USB_connectionInfo() & kUSB_vbusPresent)
      USB_handleVbusOnEvent();
    
    while(1)
    {
        BYTE i;
        // Check the USB state and directly main loop accordingly
        switch(USB_connectionState())
        {
           case ST_USB_DISCONNECTED:
                 __bis_SR_register(LPM3_bits + GIE); 	             // Enter LPM3 w/ interrupts enabled
                _nop();                                              // For Debugger
                break;
                
           case ST_USB_CONNECTED_NO_ENUM:
                break;
                
           case ST_ENUM_ACTIVE:
                __bis_SR_register(LPM0_bits + GIE); 	             // Enter LPM0 (can't do LPM3 when active)
                _NOP();                                              // For Debugger
                
                // Exit LPM on USB receive and perform a receive operation 
                if(bDataReceived_event)                              // Some data is in the buffer; begin receiving a command              
                {
                  char pieceOfString[MAX_STR_LENGTH] = "";           // Holds the new addition to the string
                  char outString[MAX_STR_LENGTH] = "";               // Holds the outgoing string
                  
                  // Add bytes in USB buffer to theCommand
                  receiveDataInBuffer((BYTE*)pieceOfString,MAX_STR_LENGTH,1);                 // Get the next piece of the string
                  strcat(wholeString,pieceOfString);                                          // Add it to the whole
                  sendData_inBackground((BYTE*)pieceOfString,strlen(pieceOfString),1,0);      // Echoes back the characters received (needed for Hyperterm)
                  
                  if(retInString(wholeString))                                                        // Has the user pressed return yet?
                  {
                    if(!(strcmp(wholeString, "LED ON")))                                              // Compare to string #1, and respond
                    {
                      TA1CTL &= ~MC_1;                                                                // Turn off Timer
                      P8OUT |= BIT2;                                                                  // Turn on LED P1.0
                      strcpy(outString,"\r\nLED is ON\r\n\r\n");                                      // Prepare the outgoing string
                      sendData_inBackground((BYTE*)outString,strlen(outString),1,0);                  // Send the response over USB
                    }
                    else if(!(strcmp(wholeString, "LED OFF")))                                        // Compare to string #2, and respond
                    {
                      TA1CTL &= ~MC_1;                                                                // Turn off Timer
                      P8OUT &= ~BIT2;                                                                 // Turn off LED P1.0
                      strcpy(outString,"\r\nLED is OFF\r\n\r\n");                                     // Prepare the outgoing string
                      sendData_inBackground((BYTE*)outString,strlen(outString),1,0);                  // Send the response over USB
                    }
                    else if(!(strcmp(wholeString, "LED TOGGLE - SLOW")))                              // Compare to string #3, and respond
                    {                      
                      TA1CTL &= ~MC_1;                                                                // Turn off Timer                      
                      TA1CCR0 = SlowToggle_Period;                                                    // Set Timer Period for slow LED toggle
                      TA1CTL |= MC_1;                                                                 // Start Timer
                      strcpy(outString,"\r\nLED is toggling slowly\r\n\r\n");                         // Prepare the outgoing string
                      sendData_inBackground((BYTE*)outString,strlen(outString),1,0);                  // Send the response over USB
                    }
                    else if(!(strcmp(wholeString, "LED TOGGLE - FAST")))                              // Compare to string #4, and respond
                    {
                      TA1CTL &= ~MC_1;                                                                // Turn off Timer                     
                      TA1CCR0 = FastToggle_Period;                                                    // Set Timer Period for fast LED toggle
                      TA1CTL |= MC_1;
                      strcpy(outString,"\r\nLED is toggling fast\r\n\r\n");                           // Prepare the outgoing string
                      sendData_inBackground((BYTE*)outString,strlen(outString),1,0);                  // Send the response over USB
                    }                    
                    else                                                                               // Handle other
                    {
                      strcpy(outString,"\r\nNo such command!\r\n\r\n");                                // Prepare the outgoing string
                      sendData_inBackground((BYTE*)outString,strlen(outString),1,0);                   // Send the response over USB
                    }                    
                    for(i=0;i<MAX_STR_LENGTH;i++)                                                       // Clear the string in preparation for the next one  
                      wholeString[i] = 0x00;
                  }
                  bDataReceived_event = FALSE;
                }
                break;
                
           case ST_ENUM_SUSPENDED:
                P8OUT &= ~BIT2;                                 // When suspended, turn off LED
                __bis_SR_register(LPM3_bits + GIE); 	        // Enter LPM3 w/ interrupts
                _NOP();
                break;
                
           case ST_ENUM_IN_PROGRESS:
                break;
           
           case ST_NOENUM_SUSPENDED:
                P8OUT &= ~BIT2; 
                __bis_SR_register(LPM3_bits + GIE);  
                _NOP();
                break;
                
           case ST_ERROR:
                _NOP();
                break;
                
           default:;
        } 
    
    }  // while(1) 
} //main()

/*----------------------------------------------------------------------------+
| System Initialization Routines                                              |
+----------------------------------------------------------------------------*/

VOID Init_Clock(VOID)
{
    //Initialization of clock module
    if (USB_PLL_XT == 2)
    {
        //P5SEL |= 0x0C;                        // enable XT2 pins for F5529
		P5SEL |= ((1<<2)|(1<<3));                        // enable XT2 pins for F5519
        UCSCTL5 |= DIVM__2;
        __delay_cycles(32000);
        
        //for F5529
        // use REFO for FLL and ACLK
        UCSCTL3 = (UCSCTL3 & ~(SELREF_7)) | (SELREF__REFOCLK);
        UCSCTL4 = (UCSCTL4 & ~(SELA_7)) | (SELA__REFOCLK);
        
        Init_FLL(USB_MCLK_FREQ/1000, USB_MCLK_FREQ/32768);             // set FLL (DCOCLK)
        XT2_Start();
    }
    else
    {
        //P5SEL |= 0x10;                    // enable XT1 pins
		P5SEL |= ((1<<4)|(1<<5));
        UCSCTL3 = SELREF__REFOCLK;            // run FLL mit REF_O clock
        UCSCTL4 = (UCSCTL4 & ~(SELA_7)) | (SELA__REFOCLK); // set ACLK = REFO
        Init_FLL(USB_MCLK_FREQ/1000, USB_MCLK_FREQ/32768);       // set FLL (DCOCLK)
        XT1_Start();
    }
    //  SFRIE1 |= OFIFG;                     // Enable OscFault ISR
}

//----------------------------------------------------------------------------

VOID Init_Ports(VOID)
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


//----------------------------------------------------------------------------

VOID Init_StartUp(VOID)
{
    __disable_interrupt();               // Disable global interrupts
    
    Init_Ports();                        // Init ports (do first ports because clocks do change ports)
    SetVCore(3);                         // USB core requires the VCore set to 1.8 volt, independ of CPU clock frequency
    Init_Clock();

    __enable_interrupt();                // enable global interrupts
}


#pragma vector = UNMI_VECTOR
__interrupt VOID UNMI_ISR(VOID)
{
    switch (__even_in_range(SYSUNIV, SYSUNIV_BUSIFG))
    {
    case SYSUNIV_NONE:
      __no_operation();
      break;
    case SYSUNIV_NMIIFG:
      __no_operation();
      break;
    case SYSUNIV_OFIFG:
      UCSCTL7 &= ~(DCOFFG+XT1LFOFFG+XT1HFOFFG+XT2OFFG); // Clear OSC flaut Flags fault flags
      SFRIFG1 &= ~OFIFG;                                // Clear OFIFG fault flag
      break;
    case SYSUNIV_ACCVIFG:
      __no_operation();
      break;
    case SYSUNIV_BUSIFG:

      // If bus error occured - the cleaning of flag and re-initializing of USB is required.
      SYSBERRIV = 0;            // clear bus error flag
      USB_disable();            // Disable
    }
}

// TimerA Init 
VOID Init_TimerA1(VOID)
{
  TA1CCTL0 = CCIE;                          // CCR0 interrupt enabled
  TA1CTL = TASSEL_1 + TACLR;                // ACLK, clear TAR
}

// This function returns true if there's an 0x0D character in the string; and if so, 
// it trims the 0x0D and anything that had followed it.  
BYTE retInString(char* string)
{
  BYTE retPos=0,i,len;
  char tempStr[MAX_STR_LENGTH] = "";
  
  strncpy(tempStr,string,strlen(string));     // Make a copy of the string
  len = strlen(tempStr);    
  while((tempStr[retPos] != 0x0A) && (tempStr[retPos] != 0x0D) && (retPos++ < len)); // Find 0x0D; if not found, retPos ends up at len
  
  if((retPos<len) && (tempStr[retPos] == 0x0D))                          // If 0x0D was actually found...
  {
    for(i=0;i<MAX_STR_LENGTH;i++)             // Empty the buffer
      string[i] = 0x00;
    strncpy(string,tempStr,retPos);           // ...trim the input string to just before 0x0D
    return TRUE;                              // ...and tell the calling function that we did so
  }
  
  else if((retPos<len) && (tempStr[retPos] == 0x0A))                                // If 0x0D was actually found...
  {
    for(i=0;i<MAX_STR_LENGTH;i++)             // Empty the buffer
      string[i] = 0x00;
    strncpy(string,tempStr,retPos);           // ...trim the input string to just before 0x0D
    return TRUE;                              // ...and tell the calling function that we did so
  }
  else if (tempStr[retPos] == 0x0D)
    {
    for(i=0;i<MAX_STR_LENGTH;i++)             // Empty the buffer
      string[i] = 0x00;
    strncpy(string,tempStr,retPos);           // ...trim the input string to just before 0x0D
    return TRUE;                              // ...and tell the calling function that we did so
  }
    
  else if (retPos<len)
     {
    for(i=0;i<MAX_STR_LENGTH;i++)             // Empty the buffer
      string[i] = 0x00;
    strncpy(string,tempStr,retPos);           // ...trim the input string to just before 0x0D
    return TRUE;                              // ...and tell the calling function that we did so
  }
 
  return FALSE;                               // Otherwise, it wasn't found
}


// Timer1 A0 interrupt service routine
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void)
{
  P8OUT ^= BIT2;                            // Toggle LED P1.0
}

/*----------------------------------------------------------------------------+
| End of source file                                                          |
+----------------------------------------------------------------------------*/
/*------------------------ Nothing Below This Line --------------------------*/
