/**
 * @file master.cpp
 * @author Daniel Dew
 * @brief Contains implementation for the master serial device 
 * @version 0.1
 * @date 2026-04-16
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "smart-serial/master.hpp"
#include "crc.hpp"
#include "error.h"
#include "frame.hpp"
#include "smart-serial/clock/IClock.hpp"

#include <cstdint>

using namespace Smart_serial;
using namespace Smart_serial::Clock;
using namespace Smart_serial::Frame;

// Initialise constants and references
Master::Master(I_port& port,
               I_clock& clock_,
               const uint8_t slave_addr,
               const uint32_t default_timeout)
               : serial_port(port), clock(clock_), slave_address(slave_addr), DEFAULT_TIMEOUT(default_timeout) {}
uint32_t Master::read_raw_frame(Raw_frame* const raw_frame_out, uint32_t timeout=0) {
    uint32_t result = S_SERIAL_ERR;
    if (timeout == 0U) {
        timeout = DEFAULT_TIMEOUT;
    }
    if (raw_frame_out != NULL) {
        raw_frame_out->length = 0;
        const uint32_t start_time = clock.millis();
        uint32_t time;
        for (time=clock.millis(); (time-start_time) < timeout; time=clock.millis()) {
            int32_t read_byte = serial_port.read_byte();
            if (read_byte == start_byte) {
                raw_frame_out->data[raw_frame_out->length++] = static_cast<uint8_t>(read_byte);
                break;
            }
            clock.delay(5U);
        }
        uint8_t payload_len = 0U;
        for (time=clock.millis(); (time-start_time) < timeout; time=clock.millis()) {
            int32_t read_byte = serial_port.read_byte();
            if (read_byte != S_SERIAL_ERR) {
                raw_frame_out->data[raw_frame_out->length] = static_cast<uint8_t>(read_byte);
                if (raw_frame_out->length == HEADER_SIZE-1U) {
                    payload_len = static_cast<uint8_t>(read_byte);
                    raw_frame_out->length++;
                    break;
                }
                raw_frame_out->length++;
            }
            clock.delay(5U);
        }
        for (time=clock.millis(); (time-start_time) < timeout; time=clock.millis()) {
            int32_t read_byte = serial_port.read_byte();
            if (read_byte != S_SERIAL_ERR_BYTE) {
                raw_frame_out->data[raw_frame_out->length++] = static_cast<uint8_t>(read_byte);
            }
            if (raw_frame_out->length >= (HEADER_SIZE + payload_len + CRC::CRC_LENGTH)) {
                break;
            }
            else { clock.delay(5U); }
        }
        if (clock.millis() - start_time < timeout) {
            result = 1;
        }
    }
    return result;
}


