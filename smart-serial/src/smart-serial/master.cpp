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
using namespace Smart_serial::CRC;

// Initialise constants and references
Master::Master(I_port& port,
               I_clock& clock_,
               const uint8_t slave_addr,
               const uint32_t default_timeout)
               : serial_port(port), clock(clock_), slave_address(slave_addr), DEFAULT_TIMEOUT(default_timeout) {}

int32_t Master::read_raw_frame(Raw_frame* const raw_frame_out, const uint32_t timeout) {
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
                // Last header byte is payload length — extend expected_total accordingly
                payload_len = raw_frame_out->data[HEADER_SIZE - 1U];
                expected_total += static_cast<size_t>(payload_len);
                stage2_done = true; // stage 2 done
                break;
            }
        }

        // Stage 3 — read payload and CRC bytes until frame is complete
        while (stage2_done &&
              ((clock.millis() - start_time) < effective_timeout) &&
              (raw_frame_out->length < S_SERIAL_MAX_FRAME_BYTES)) {
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

Receive_result Master::receive_packet(Frame::Frame* const frame_out,
                                      const uint32_t timeout,
                                      bool check_crc) {
    Receive_result result = ERR_PROCESS;
    if (frame_out != NULL) {
        Raw_frame raw_frame;
        const int32_t read_raw_result = read_raw_frame(&raw_frame, timeout);
        if (read_raw_result == 1) {
            // Find crc index (last 2 bytes)
            const uint32_t crc_offset = static_cast<uint32_t>(raw_frame.length) - CRC_LENGTH - 1U;
            // read crc from buffer and compute expected
            const uint16_t read_crc = extract_crc16(raw_frame.data, crc_offset);
            const uint16_t expected_crc = compute_crc16(raw_frame.data, crc_offset);
            if (read_crc == expected_crc) {
                raw_frame.length = crc_offset;
                // parse the raw frame into decomposed frame
                const int32_t parse_result = parse_frame(frame_out, &raw_frame);
                if (parse_result == 1) {
                    if (frame_out->header.command == NACK) {
                        // Slave has sent a NACK
                        result = ERR_NACK;
                    } else {
                        result = SUCCESS;
                    }
                }
            } else{
                // CRC check has failed
                result = ERR_CRC;
            }
        } else if (read_raw_result == S_SERIAL_ERR_TIMEOUT) {
            // Process failed due to timeout
            result = ERR_TIMEOUT;
        }
    }
    return result;
}

int32_t Master::send_frame(const Frame::Frame* const frame) {
    uint32_t result = S_SERIAL_ERR;
    if (frame != NULL){
        Raw_frame raw_frame;
        // Dump frame into raw frame (data, len)
        int32_t dump_result = dump_frame(&raw_frame, frame);
        if (dump_result != S_SERIAL_ERR) {
            // Compute crc for data
            int16_t crc = compute_crc16(&raw_frame);
            if (crc != S_SERIAL_ERR_2_BYTE) {
                // Append crc to data and write to tx buffer
                int32_t append_res = append_crc16(&raw_frame, S_SERIAL_MAX_FRAME_BYTES, raw_frame.length, crc);
                if (append_res == 1) {          
                    int32_t write_res = serial_port.write(raw_frame.data, raw_frame.length);
                    result = write_res;
                }
            }
        }
    }
    return result;
}

int32_t Master::send_string(const char* const str, const uint8_t cmd_byte) {
    // Converts string to uint8_t buffer and sends that
    int32_t result = S_SERIAL_ERR;
    if (str != NULL) {
        const size_t payload_len = strlen(str);
        const uint8_t* buf = reinterpret_cast<const uint8_t*>(str[0U]);
        result = send_bytes(buf, cmd_byte);
    }
    return result;
}

int32_t Master::send_bytes(const uint8_t* const data, const uint8_t cmd_byte) {
    // Builds frame from payload buffer and sends
    int32_t result = S_SERIAL_ERR;
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
        if (build_frame_res == 1) {
            int32_t transmit_res = send_frame(&frame);
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
    // Send bytes, read response
    Receive_result result = ERR_PROCESS;
    if ((frame_out != NULL) && (buf != NULL)) {
        int32_t send_res = send_bytes(buf, cmd_byte);
        if (send_res == 1) {
            result = receive_packet(frame_out, timeout);
        }
    }
    return result;
}

Receive_result Master::transact(
    Frame::Frame* const frame_out,
    const Frame::Frame* const frame_in,
    const uint32_t timeout
) {
    // sends bytes (from payload of frame) and reads resoonse
    Receive_result result = ERR_PROCESS;
    if ((frame_out != NULL) && (frame_in != NULL)) {
        result = transact(
            frame_out,
            frame_in->payload,
            frame_in->header.command,
            timeout
        );
    }
    return result;
}
Receive_result Master::transact(
    Frame::Frame* const frame_out,
    const char* const str,
    const uint8_t cmd_byte,
    const uint32_t timeout
) {
    // Delegates to overload with a converted string to uint8_t buffer
    const Receive_result result = transact(
        frame_out,
        reinterpret_cast<const uint8_t*>(str[0]),
        cmd_byte,
        timeout
    );
    return result;
}

Receive_result Master::handshake(const uint32_t attempts, const uint32_t timeout_per) {
    Receive_result result = ERR_PROCESS;
    Frame::Frame handshake_frame;
    // Default ACK frame with no payload and ACK cmd
    int32_t build_res = build_frame(
        &handshake_frame,
        start_byte,
        this_address,
        slave_address,
        ACK,
        nullptr,
        0U
    );
    if (build_res == 1) {
        // Attempts to transact the ACK frame multiple times
        for (uint32_t attempt_num=0U; attempt_num < attempts; ++attempt_num) {
            Frame::Frame ack_frame;
            result = transact(&ack_frame, &handshake_frame, timeout_per);
            if (result == SUCCESS && ack_frame.header.command == ACK) {
                break;
            } else if (result == SUCCESS) {
                result = ERR_UNEXPECTED_CMD;
            }
        }
        if (result == ERR_PROCESS) {
            result = ERR_TIMEOUT;
        }
    }
    return result;
}