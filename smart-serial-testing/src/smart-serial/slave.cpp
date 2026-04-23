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

#include "smart-serial/error.h"
#include "smart-serial/port/iport.hpp"
#include "smart-serial/types.h"
#include "smart-serial/slave.hpp"
#include "smart-serial/frame.hpp"
#include "smart-serial/crc.hpp"
#include <cstdint>

using namespace Smart_serial;
using namespace Smart_serial::CRC;

Slave::Slave(
    I_port& port, 
    Clock::I_clock& clock, 
    uint8_t this_addr,
    uint32_t default_timeout
) : serial_port(port), clock(clock), this_address(this_addr), DEFAULT_TIMEOUT(default_timeout) {}

Slave::~Slave(){}

Receive_result Slave::receive_request(Frame::Frame* const frame_out, uint32_t timeout) {
    Receive_result result = ERR_PROCESS;
    if (frame_out != NULL) {
        Frame::Raw_frame raw_frame;
        int32_t read_result = read_raw_frame(&raw_frame, timeout);
        if (read_result == 1) {
            int32_t valid_crc = validate_crc(&raw_frame);
            int32_t build_frame_res = Frame::parse_frame(frame_out, &raw_frame);
            if (valid_crc == ERR_CRC) {
                result = ERR_CRC;
            }
            else if ((build_frame_res == 1) && (valid_crc != ERR_PROCESS)) {
                result = (frame_out->header.command == NACK) ? ERR_NACK : SUCCESS;
                result = (frame_out->header.to_address != this_address) ? ERR_WRONG_ADDRESS : SUCCESS;
                if (
                    frame_out->header.command == ACK &&
                    frame_out->header.payload_len == 0U &&
                    result > 0U &&
                    auto_handshake
                ) {
                    int32_t shake_res = send_response(frame_out);
                    result = (shake_res == 1) ? HANDLED : ERR_PROCESS;
                }
            }
        } else if (read_result == S_SERIAL_ERR_TIMEOUT) {
            result = ERR_TIMEOUT;
        }
    }
    return result;
}

void Slave::set_start_byte(const uint8_t byte) {
    start_byte = byte;
}
void Slave::set_this_address(const uint8_t byte) {
    this_address = byte;
}
void Slave::set_auto_handshake(const bool auto_shake) {
    auto_handshake = auto_shake;
}

int32_t Slave::send_response(const Frame::Frame* frame) {
    int32_t result = S_SERIAL_ERR;
    if (frame != NULL){
        Frame::Raw_frame raw_frame;
        // Dump frame into raw frame (data, len)
        int32_t dump_result = dump_frame(&raw_frame, frame);
        if (dump_result != S_SERIAL_ERR) {
            // Compute crc for data
            int16_t crc = compute_crc16(&raw_frame);
            if (crc != S_SERIAL_ERR_2_BYTE) {
                // Append crc to data and write to tx buffer
                int32_t append_res = append_crc16(&raw_frame, Frame::S_SERIAL_MAX_FRAME_BYTES, raw_frame.length, crc);
                if (append_res == 1) {
                    int32_t write_res = serial_port.write(raw_frame.data, raw_frame.length);
                    result = (write_res < 0) ? write_res : 1;
                }
            }
        }
    }
    return result;
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
        size_t expected_total = Frame::HEADER_SIZE + CRC_LENGTH;

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
              (raw_frame_out->length < Frame::S_SERIAL_MAX_FRAME_BYTES) &&
              ((clock.millis() - start_time) < effective_timeout)) {
                  const int32_t read_byte = serial_port.read_byte();
                  if (read_byte < 0) {
                      // No byte, continue
                      clock.delay(1U);
                      continue;
                    }
                    // standard header byte
                    raw_frame_out->data[raw_frame_out->length++] = static_cast<uint8_t>(read_byte);
                    
                    if (raw_frame_out->length == Frame::HEADER_SIZE) {
                        if (raw_frame_out->data[2U] != this_address) {
                            result = ERR_WRONG_ADDRESS;
                        } else {
                            // Last header byte is payload length — extend expected_total accordingly
                            payload_len = raw_frame_out->data[Frame::HEADER_SIZE - 1U];
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

int32_t Slave::validate_crc(const Frame::Raw_frame* raw_frame) {
    int32_t result = ERR_PROCESS;
    if (raw_frame != NULL) {
        // Calculate expected crc and actual crc from raw frame
        const size_t crc_offset = raw_frame->length - CRC_LENGTH;
        const int16_t expected_crc = compute_crc16(raw_frame->data, crc_offset);
        const int16_t actual_crc = extract_crc16(raw_frame->data, crc_offset);

        if ((expected_crc != S_SERIAL_ERR_2_BYTE) && (actual_crc != S_SERIAL_ERR_2_BYTE)) {
            result = (expected_crc == actual_crc) ? 1 : ERR_CRC;
        }
    }
    return result;
}