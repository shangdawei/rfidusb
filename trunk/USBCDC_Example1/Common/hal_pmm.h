//****************************************************************************//
// Function Library for setting the PMM
//    File: hal_pmm.h
//
//    Texas Instruments
//
//    Version 1.1
//    07/27/09
// 
//****************************************************************************////====================================================================


#ifndef __PMM
#define __PMM



//====================================================================
/**
  * Set the VCore to a new level
  *
  * \param level       PMM level ID
  */
void SetVCore (unsigned char level);

//====================================================================
/**
  * Set the VCore to a new higher level
  *
  * \param level       PMM level ID
  */
void SetVCoreUp (unsigned char level);

//====================================================================
/**
  * Set the VCore to a new Lower level
  *
  * \param level       PMM level ID
  */
void SetVCoreDown (unsigned char level);

#endif /* __PMM */
