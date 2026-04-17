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
uint32_t Master::read_raw_frame(Raw_frame* const raw_frame_out, const uint32_t timeout) {
    uint32_t result = S_SERIAL_ERR;
    const uint32_t effective_timeout = (timeout == 0U) ? DEFAULT_TIMEOUT : timeout;

    if (raw_frame_out != NULL) {
        raw_frame_out->length = 0U;
        const uint32_t start_time = clock.millis();
        bool stage1_done = false;
        bool stage2_done = false;
        uint8_t payload_len = 0U;
        // Minimum frame size — updated in stage 2 once payload length is known
        size_t expected_total = HEADER_SIZE + CRC::CRC_LENGTH;

        // Stage 1 — scan incoming bytes until start byte is found
        while ((clock.millis() - start_time) < effective_timeout) {
            const int32_t read_byte = serial_port.read_byte();
            if (read_byte < 0) {
                clock.delay(1U); // nothing available, yield
                continue;
            }
            else if (static_cast<uint8_t>(read_byte) == start_byte) {
                raw_frame_out->data[raw_frame_out->length++] = start_byte;
                stage1_done = true;
                break;
            }
            // non-start byte — discard and keep scanning
        }

        // Stage 2 — read remaining header bytes until payload length is known
        while (stage1_done &&
              (raw_frame_out->length < S_SERIAL_MAX_FRAME_BYTES) &&
              ((clock.millis() - start_time) < effective_timeout)) {
            const int32_t read_byte = serial_port.read_byte();
            if (read_byte < 0) {
                clock.delay(1U);
                continue;
            }
            raw_frame_out->data[raw_frame_out->length++] = static_cast<uint8_t>(read_byte);

            if (raw_frame_out->length == HEADER_SIZE) {
                // Last header byte is payload length — extend expected_total accordingly
                payload_len = raw_frame_out->data[HEADER_SIZE - 1U];
                expected_total += static_cast<size_t>(payload_len);
                stage2_done = true;
                break;
            }
        }

        // Stage 3 — read payload and CRC bytes until frame is complete
        while (stage2_done &&
              ((clock.millis() - start_time) < effective_timeout) &&
              (raw_frame_out->length < S_SERIAL_MAX_FRAME_BYTES)) {
            const int32_t read_byte = serial_port.read_byte();
            if (read_byte < 0) {
                clock.delay(1U);
                continue;
            }
            raw_frame_out->data[raw_frame_out->length++] = static_cast<uint8_t>(read_byte);

            if (raw_frame_out->length >= expected_total) {
                result = 1U; // frame complete
                break;
            }
        }
    }
    return result;
}


