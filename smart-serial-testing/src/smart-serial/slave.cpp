/**
 * @file slave.cpp
 * @author Daniel Dew
 * @brief Contains implementations of slave device class
 * @version 0.1
 * @date 2026-04-19
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "smart-serial/port/iport.hpp"
#include "smart-serial/types.h"
#include "smart-serial/slave.hpp"
#include "smart-serial/frame.hpp"
#include "smart-serial/crc.hpp"

using namespace Smart_serial;
using namespace Smart_serial::Frame;
using namespace Smart_serial::CRC;

Slave::Slave(
    I_port& port, 
    Clock::I_clock& clock, 
    uint8_t this_addr,
    uint32_t default_timeout
) : serial_port(port), clock(clock), this_address(this_addr), DEFAULT_TIMEOUT(default_timeout) {}

Slave::~Slave(){}

void Slave::set_start_byte(const uint8_t byte) {
    start_byte = byte;
}
void Slave::set_this_address(const uint8_t byte) {
    this_address = byte;
}
void Slave::set_auto_handshake(const bool auto_shake) {
    auto_handshake = auto_shake;
}

int32_t Slave::read_raw_frame(Frame::Raw_frame* const raw_frame_out, uint32_t timeout) {
    int32_t result = S_SERIAL_ERR;
    // Use default timeout if otherwise specified
    const uint32_t effective_timeout = (timeout == 0U) ? DEFAULT_TIMEOUT : timeout;

    if (raw_frame_out != NULL) {
        // Initialise flags and values
        raw_frame_out->length = 0U;
        const uint32_t start_time = clock.millis();
        bool stage1_done = false;
        bool stage2_done = false;
        uint8_t payload_len = 0U;
        // Minimum frame size — updated in stage 2 once payload length is known
        size_t expected_total = HEADER_SIZE + CRC_LENGTH;

        // Stage 1 — scan incoming bytes until start byte is found
        while ((clock.millis() - start_time) < effective_timeout) {
            const int32_t read_byte = serial_port.read_byte();
            if (read_byte < 0) {
                clock.delay(1U); // nothing available, yield
                continue;
            }
            else if (static_cast<uint8_t>(read_byte) == start_byte) {
                // A packet has begun
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
                      // No byte, continue
                      clock.delay(1U);
                      continue;
                    }
                    // standard header byte
                    raw_frame_out->data[raw_frame_out->length++] = static_cast<uint8_t>(read_byte);
                    
                    if (raw_frame_out->length == HEADER_SIZE) {
                        if (raw_frame_out->data[2U] != this_address) {
                            result = ERR_WRONG_ADDRESS;
                        } else {
                            // Last header byte is payload length — extend expected_total accordingly
                            payload_len = raw_frame_out->data[HEADER_SIZE - 1U];
                            expected_total += static_cast<size_t>(payload_len);
                            stage2_done = true; // stage 2 done
                            break;
                        }
                    }
        }
                
        // Stage 3 — read payload and CRC bytes until frame is complete
        while (stage2_done &&
              ((clock.millis() - start_time) < effective_timeout) &&
              (raw_frame_out->length < expected_total)) {
            const int32_t read_byte = serial_port.read_byte();
            if (read_byte < 0) {
                // No byte, continue
                clock.delay(1U);
                continue;
            }
            raw_frame_out->data[raw_frame_out->length++] = static_cast<uint8_t>(read_byte);

            if (raw_frame_out->length >= expected_total) {
                result = 1; // raw frame complete
                break;
            }
        }
        if (result != 1) {
            // No success due to timeout
            result = S_SERIAL_ERR_TIMEOUT;
        }
    }
    return result;
}