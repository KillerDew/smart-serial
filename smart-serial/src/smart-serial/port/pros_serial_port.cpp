/**
 * @file pros_serial_port.cpp
 * @author Daniel Dew
 * @brief Implementation of Pros_serial_port class, which wraps PROS serial port functionality in a class that implements the I_port interface.
 ** Static casting only. AV Rule 185.
 * @version 0.1
 * @date 2026-04-09
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "smart-serial/port/pros_serial_port.hpp"
#include "smart-serial/error.h"
#include "pros/serial.hpp"
#include "pros/error.h"
#include <cstdint>

using namespace Smart_serial;

Pros_serial_port::Pros_serial_port(const uint32_t smart_port_num, const uint32_t baud_rate=115200)
    : serial_port(static_cast<int32_t>(smart_port_num), static_cast<int32_t>(baud_rate))
    { /* Port is now open */ }

Pros_serial_port::~Pros_serial_port() {} // PROS handles cleanup.

// Write is delegated to pros::Serial::write(), which returns PROS_ERR on error. We convert this to S_SERIAL_ERR to insulate callers from PROS-specific error codes.
int32_t Pros_serial_port::write(uint8_t *buf, uint16_t len) {

    const int32_t result = serial_port.write(static_cast<uint8_t*>(buf), static_cast<size_t>(len));

    return (result == PROS_ERR) ? S_SERIAL_ERR : result; // Convert PROS_ERR to S_SERIAL_ERR, otherwise return bytes written.
}

// Read byte is delegated to pros::Serial::read_byte(), which returns PROS_ERR if no byte is available. We convert this to S_SERIAL_ERR to insulate callers from PROS-specific error codes.
int32_t Pros_serial_port::read_byte() {
    const int32_t result = serial_port.read_byte();
    return (result == PROS_ERR) ? S_SERIAL_ERR : result; // Convert PROS_ERR to S_SERIAL_ERR, otherwise return byte read.
}

void Pros_serial_port::flush_rx() {
    // Read bytes until no more are available, bounded to prevent infinite loop. AV Rule 113.
    for (uint16_t i = 0U; i < MAX_FLUSH_ITERS; ++i) {
        if (read_byte() == S_SERIAL_ERR) {
            break; // No more bytes available, exit loop.
        }
    }
}