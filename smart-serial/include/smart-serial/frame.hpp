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

#define S_SERIAL_MAX_FRAME_BYTES (256)
namespace Smart_serial { namespace Frame {

    /**
     * @brief Struct containing data buffer and length of data in buffer
     */
    struct Raw_frame {
        uint8_t data[S_SERIAL_MAX_FRAME_BYTES];
        size_t length;
    };

    struct Frame_header {
        uint8_t start_byte;
        uint8_t address;
        uint8_t command;
        uint8_t payload_len;
    };

    struct Frame {
        Frame_header header;
        uint8_t payload[];
    };

    Frame build_frame(const uint8_t start_byte, 
                      const uint8_t addr,
                      const uint8_t cmd,
                      const uint8_t*
                      const payload,
                      const size_t len);

} // namespace Frame
} // namespace Smart_serial

#endif // SMART_SERIAL_FRAME_HPP
