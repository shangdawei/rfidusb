//-----------------------------------------------------------
//Header file for automatic tag detection

#include <msp430f5519.h> 	
#include <stdio.h>
#include "hardware.h"
#include "globals.h"
#include "host.h"
#include "parallel.h"
#include "SPI.h"
#include "anticollision.h"
#include "14443.h"
#include "tiris.h"
#include "epc.h"
//--------------------------------------------------------

unsigned char FindTags(unsigned char protocol);
extern unsigned char protocol;

