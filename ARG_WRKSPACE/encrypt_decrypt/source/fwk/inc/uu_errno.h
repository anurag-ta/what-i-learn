/*************************************************************************
**                                                                      **
** File name :  uu_errno.h                                              **
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

#ifndef __UU_ERRNO_H__
#define __UU_ERRNO_H__

/*
 * NOTE: Always use 32-bit signed integer to return / capture the error numbers.
 */


/* Success, every where */
#define UU_SUCCESS               0

/*
 * Generic error code to indicate failure.
 * Use specific error code, whenever possible.
 */
#define UU_FAILURE              -1   /* 0xFFFFFFFF */


/*
 * Error number base for generic errors.
 * Each project must maintain its own additional error numbers base file.
 * Module-specific error codes start with module's base error numbers.
 */
#define UU_GEN_ERR_BASE       0x00F00000

/*
 * Invalid parameter passed to the function
 */
#define UU_ERR_INVALID_PARAM  -(UU_GEN_ERR_BASE + 1)


#endif /* __UU_ERRNO_H__ */

