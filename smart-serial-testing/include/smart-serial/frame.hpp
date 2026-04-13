/**
 * @file frame.hpp
 * @author Daniel Dew
 * @brief Header containing definitions related to frames.
 * @version 0.1
 * @date 2026-04-12
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef SMART_SERIAL_FRAME_HPP
#define SMART_SERIAL_FRAME_HPP

#include <cstddef>
#include <cstdint>

namespace Smart_serial { namespace Frame {

    static const size_t S_SERIAL_MAX_FRAME_BYTES   = 256U;
    static const size_t S_SERIAL_MAX_PAYLOAD_BYTES = 252U;
    static const size_t HEADER_SIZE = 5U;

    /**
     * @brief Struct containing data buffer and length of data in buffer
     */
    struct Raw_frame {
        uint8_t data[S_SERIAL_MAX_FRAME_BYTES];
        size_t length;
    };

    /**
     * @brief Header for a frame
     * Contains surrounding data of a frame
     */
    struct Frame_header {
        uint8_t start_byte;
        uint8_t from_address;
        uint8_t to_address;
        uint8_t command;
        uint8_t payload_len;
    };

    /**
     * @brief Frame struct, contains header and payload
     */
    struct Frame {
        Frame_header header;
        uint8_t payload[S_SERIAL_MAX_PAYLOAD_BYTES];
    };

    /**
     * @brief Builds a frame from seperated data
     * 
     * @param frame_out Pointer to frame to write data to
     * @param start_byte The start byte to use. Defualts to 0x02
     * @param addr Address byte of intended recipient
     * @param cmd Command byte to use
     * @param payload array containing payload data
     * @param payload_len Length of payload
     * @return uint32_t 1 if succesful, S_SERIAL_ERR if not. 
     */
    uint32_t build_frame(Frame* const frame_out,
                         const uint8_t start_byte, 
                         const uint8_t from_addr,
                         const uint8_t to_addr,
                         const uint8_t cmd,
                         const uint8_t* const payload,
                         const size_t payload_len);
    
    /**
     * @brief Dumps a frame into a raw frame
     * 
     * @param raw_frame_out The raw frame to write to
     * @param frame The frame to write from. Must not be NULL.
     * @return uint32_t 1 if succesful, S_SERIAL_ERR if not
     */
    uint32_t dump_frame(Raw_frame* const raw_frame_out, const Frame* const frame);

    /**
     * @brief Parse a raw frame to decomposed Frame
     * 
     * @param frame_out The frame to write to
     * @param raw_frame The raw frame to parse. Must not be NULL.
     * @return uint32_t 1 if successful, S_SERIAL_ERR if not
     */
    uint32_t parse_frame(Frame* const frame_out, const Raw_frame* const raw_frame_in);
    /**
     * @brief Parse a raw frame to decomposed Frame
     * 
     * @param frame_out The frame to write to
     * @param buf The buffer to decompose. Must not be NULL.
     * @param offset The start index of the frame in the buffer
     * @param len The length of the frame
     * @return uint32_t 1 if successful, S_SERIAL_ERR if not
     */
    uint32_t parse_frame(Frame* const frame_out, const uint8_t* const buf, const uint16_t offset, const size_t len);

} // namespace Frame
} // namespace Smart_serial

#endif // SMART_SERIAL_FRAME_HPP
