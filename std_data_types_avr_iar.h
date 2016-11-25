/******************************************************************************
*   std_data_types_avr_iar.h
*
*   This header file typedef's a number of common data types into symbols
*   whose bit-widths can be remembered more easily.
*
*	This file is specifically for the IAR AVR compiler
*
*   Written By: Brad Sundstrom
*
*   I wrote this because it can be confusing when working with 8-bit and 32-bit
*   processors at the same time!
******************************************************************************/
#ifndef STD_DATA_TYPES_H
#define STD_DATA_TYPES_H

typedef char* string;
typedef unsigned char BYTE;
typedef unsigned char UINT8;
typedef signed char INT8;
typedef unsigned short UINT16;
typedef signed short INT16;
typedef unsigned long UINT32;
typedef signed long INT32;
//typedef unsigned long UINT64; not available for avr
//typedef signed long INT64; not available for avr
#endif
