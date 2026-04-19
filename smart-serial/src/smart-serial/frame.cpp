/**
 * @file frame.cpp
 * @author Daniel Dew
 * @brief Contains implementations of frame related utilities
 * @version 0.1
 * @date 2026-04-13
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "smart-serial/error.h"
#include "smart-serial/crc.hpp"
#include "smart-serial/frame.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

using namespace Smart_serial;

uint32_t Frame::build_frame(Frame* const frame_out,
                            const uint8_t start_byte,
                            const uint8_t from_addr,
                            const uint8_t to_addr,
                            const uint8_t cmd,
                            const uint8_t* const payload,
                            const size_t payload_len) {
    
    uint32_t result = S_SERIAL_ERR;
    // Check if payload is not too large
    if ((frame_out != NULL) && (payload_len <= S_SERIAL_MAX_PAYLOAD_BYTES)) {
        // Assign header values
        frame_out->header.start_byte = start_byte;
        frame_out->header.from_address = from_addr;
        frame_out->header.to_address = to_addr;
        frame_out->header.command = cmd;
        frame_out->header.payload_len = payload_len;
        // Write payload to frame
        static_cast<void>(memcpy(frame_out->payload, payload, payload_len));
        // Success
        result = 1;
    }
    return result;
}

uint32_t Frame::dump_frame(Raw_frame* const raw_frame_out, const Frame *const frame) {
    uint32_t result = S_SERIAL_ERR;
    if ((raw_frame_out !=  NULL) &&
        (frame != NULL)
    ) {
        // Assign starting bytes
        raw_frame_out->data[0U] = frame->header.start_byte;
        raw_frame_out->data[1U] = frame->header.to_address;
        raw_frame_out->data[2U] = frame->header.from_address;
        raw_frame_out->data[3U] = frame->header.command;
        raw_frame_out->data[4U] = frame->header.payload_len;
        // Append payload to buffer
        for (size_t i=0; i<frame->header.payload_len; ++i) {
            raw_frame_out->data[5U + i] = frame->payload[i];
        }
        // Calculate and assign data length
        raw_frame_out->length = frame->header.payload_len + 5U;

        result = 1;
    }
    return result;
}

uint32_t Frame::parse_frame(Frame* const frame_out, const Raw_frame* raw_frame_in) {
    uint32_t result = S_SERIAL_ERR;

    // Check for null references and if data is long enough to populate frame
    if ((frame_out != NULL) && (raw_frame_in != NULL)) {
        result = parse_frame(frame_out,
            raw_frame_in->data,
            0U,
            raw_frame_in->length);
    }
    return result;
}

uint32_t Frame::parse_frame(Frame *const frame_out,
                            const uint8_t *const buf,
                            const uint16_t offset,
                            const size_t len) {

    uint32_t result = S_SERIAL_ERR;
    // Check for null references and if data is long enough to populate frame
    if ((frame_out != NULL) && (len >= HEADER_SIZE)) {
        
        const uint8_t payload_len = buf[offset + 4U];

        if (len >= (static_cast<size_t>(offset) + HEADER_SIZE + static_cast<size_t>(payload_len) + CRC::CRC_LENGTH)){
        
            // Assign header info
            frame_out->header.start_byte = buf[offset];
            frame_out->header.to_address = buf[offset + 1U];
            frame_out->header.from_address = buf[offset + 2U];
            frame_out->header.command = buf[offset + 3U];
            frame_out->header.payload_len = payload_len;
            // Assign payload data to frame
            static_cast<void>(memcpy(frame_out->payload,
                            &buf[offset + 5U],
                            static_cast<size_t>(payload_len)));
            
            result = 1;
        }
    }
    return result;
}