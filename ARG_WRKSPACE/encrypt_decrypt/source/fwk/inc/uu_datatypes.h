/*************************************************************************
**                                                                      **
** File name :  uu_datatypes.h                                          **
**                                                                      **
** Copyright © 2013, Uurmi Systems                                     **
** All rights reserved.                                                 **
** http://www.uurmi.com                                                 **
**                                                                      **
** All information contained herein is property of Uurmi Systems        **
** unless otherwise explicitly mentioned.                               **
**                                                                      **
** The intellectual and technical concepts in this file are proprietary **
** to Uurmi Systems and may be covered by granted or in process national**
** and international patents and are protect by trade secrets and       **
** copyright law.                                                       **
**                                                                      **
** Redistribution and use in source and binary forms of the content in  **
** this file, with or without modification are not permitted unless     **
** permission is explicitly granted by Uurmi Systems.                   **
**                                                                      **
*************************************************************************/

#ifndef __UU_DATATYPES_H__
#define __UU_DATATYPES_H__

#ifdef USER_SPACE
#else
#include <linux/kernel.h>
#include <linux/module.h>

#define assert(x) 
#endif

#include <stdbool.h>

typedef void                uu_void;

typedef unsigned char       uu_uchar;
typedef char                uu_char;

typedef unsigned char       uu_uint8;
typedef unsigned short      uu_uint16;
typedef unsigned int        uu_uint32;
typedef unsigned long long int   uu_uint64;

typedef signed char         uu_int8;
typedef signed short        uu_int16;
typedef signed int          uu_int32;
typedef long long int       uu_int64;

typedef bool                uu_bool;


typedef char            uuChar;
typedef unsigned char   uuUChar;
typedef short           uuInt16;
typedef int             uuInt32;
typedef unsigned int    uuUInt32;
typedef unsigned short  uuUInt16;
typedef long long int   uuInt64;
typedef unsigned long long int  uuUInt64;
typedef void            uuVoid;
typedef int             uuSocket_t ;

#ifdef __cplusplus
#define UU_BEGIN_DECLARATIONS  extern "C" {
#define UU_END_DECLARATIONS   }
#else
#define UU_BEGIN_DECLARATIONS 
#define UU_END_DECLARATIONS 
#endif 

#define UU_FALSE               0
#define UU_TRUE                1

#define UU_NULL                NULL


#endif /* __UU_DATATYPES_H__ */

/* EOF */


