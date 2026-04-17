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

#include <cstddef>
#include <cstdint>
#include <cstring>

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

Receive_result Master::receive_packet(Frame::Frame* const frame_out,
                                      const uint32_t timeout,
                                      bool check_crc) {
    Receive_result result = ERR_PROCESS;
    Raw_frame raw_frame;
    const int32_t read_raw_result = read_raw_frame(&raw_frame, timeout);
    if (read_raw_result == 1U) {
        const uint32_t crc_offset = raw_frame.length - CRC::CRC_LENGTH - 1U;
        const uint16_t read_crc = CRC::extract_crc16(raw_frame.data, crc_offset);
        const uint16_t expected_crc = CRC::compute_crc16(raw_frame.data, crc_offset);
        if (read_crc == expected_crc) {
            const uint32_t parse_result = parse_frame(frame_out, &raw_frame);
            if (parse_result == 1U) {
                if (frame_out->header.command == NACK) {
                    result = ERR_NACK;
                } else {
                    result = SUCCESS;
                }
            }
        } else{
            result = ERR_CRC;
        }
    }
    return result;
}

uint32_t Master::send_frame(const Frame::Frame* frame) {
    uint32_t result = S_SERIAL_ERR;
    if (frame != NULL){
        Raw_frame raw_frame;
        uint32_t dump_result = dump_frame(&raw_frame, frame);
        if (dump_result != S_SERIAL_ERR) {
            int32_t write_res = serial_port.write(raw_frame.data, raw_frame.length);
            result = write_res;
        }
    }
    return result;
}

uint32_t Master::send_string(const char* const str, const uint8_t cmd_byte) {
    uint32_t result = S_SERIAL_ERR;
    if (str != NULL) {
        const size_t payload_len = strlen(str);
        const uint8_t* buf = reinterpret_cast<const uint8_t*>(str[0U]);
        result = send_bytes(buf, cmd_byte);
    }
    return result;
}

uint32_t Master::send_bytes(const uint8_t* const data, const uint8_t cmd_byte) {
    uint32_t result = S_SERIAL_ERR;
    if (data != NULL) {
        Frame::Frame frame;
        const size_t payload_len = static_cast<size_t>(sizeof(data));
        uint32_t build_frame_res = build_frame(
            &frame,
            start_byte,
            this_address,
            slave_address,
            cmd_byte,
            data,
            payload_len
        );
        if (build_frame_res == 1U) {
            uint32_t transmit_res = send_frame(&frame);
            result = transmit_res;
        }
    }
    return result;
}

Receive_result Master::transact(
    Frame::Frame* const frame_out,
    const uint8_t* const buf,
    const uint8_t cmd_byte,
    const uint32_t timeout
) {
    Receive_result result = ERR_PROCESS;
    if ((frame_out != NULL) && (buf != NULL)) {
        uint32_t send_res = send_bytes(buf, cmd_byte);
        if (send_res == 1U) {
            result = receive_packet(frame_out, timeout);
        }
    }
    return result;
}