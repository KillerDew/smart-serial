/**
 * @file types.h
 * @author Daniel Dew
 * @brief Contains definitions of common return types (used over multiple implementations)
 * @version 0.1
 * @date 2026-04-19
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef S_SERIAL_TYPES_H
#define S_SERIAL_TYPES_H

#include "error.h"
#include <cstdint>

namespace Smart_serial {


enum Receive_result {
        SUCCESS = 1,
        ERR_NACK = 0,
        ERR_CRC = -1,
        ERR_TIMEOUT = -2,
        ERR_UNEXPECTED_CMD = -3,
        ERR_PROCESS = S_SERIAL_ERR
};

enum Default_cmds {
    ACK = 0x06,
    NACK = 0x15
};

}
#endif // S_SERIAL_TYPES_H