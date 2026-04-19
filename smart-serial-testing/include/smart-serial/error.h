/**
 * @file error.h
 * @author Daniel Dew
 * @brief Contains macro definitions for errors in smart-serial.
 * @version 0.1
 * @date 2026-04-09
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef SMART_SERIAL_ERROR_H
#define SMART_SERIAL_ERROR_H

#include "limits.h"

// Different Byte Size Errors

/// @brief
/// Return This on Byte Sized Return Error
#define S_SERIAL_ERR_BYTE (INT8_MIN)

/// @brief
/// Return This on 2 Byte Sized Return Error
#define S_SERIAL_ERR_2_BYTE (INT16_MIN)

/// @brief
/// Return This on 4 Byte Sized Return Error
#define S_SERIAL_ERR (INT32_MIN)

/// @brief
/// Return This on 8 Byte Sized Return Error
#define S_SERIAL_ERR_F (-INFINITY)

/// @brief
/// Return this on any errors to do with timeouts
#define S_SERIAL_ERR_TIMEOUT (-2)

/// @brief
/// Return This on Success (1)
#define S_SERIAL_SUCCESS (1)

#endif // SMART_SERIAL_ERROR_H