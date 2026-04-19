/**
 * @file crc.hpp
 * @author Daniel Dew
 * @brief Contains definition for CRC related utilities.
 * @version 0.1
 * @date 2026-04-11
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef SMART_SERIAL_CRC_HPP
#define SMART_SERIAL_CRC_HPP

#include "smart-serial/frame.hpp"
#include <cstdint>

namespace Smart_serial { namespace CRC {

    static const size_t CRC_LENGTH = 2U;

    /**
     * @brief Computes a CRC16 checksum for the given data buffer and length.
     * 
     * @param data data to compute CRC for
     * @param length length of data buffer
     * @return uint16_t the 16 bit CRC checksum
     */
    uint16_t compute_crc16(const uint8_t* const data, uint16_t length);

    uint16_t compute_crc16(const Frame::Raw_frame* const raw_frame);

    /**
     * @brief Extracts the 16 bit CRC
     * 
     * @param data data to extract CRC from
     * @param offset index at which the CRC is located in the data buffer
     * @return uint16_t the extracted 16 bit CRC
     */
    uint16_t extract_crc16(const uint8_t* const data, uint16_t offset);

    /**
     * @brief appends a CRC to the buffer with low byte first
     * 
     * @param buf buffer to append CRC to
     * @param cap The maximum capacity of the buffer incl. CRC
     * @param offset index at which to insert the CRC
     * @param crc 16 bit CRC to append
     * @return uint16_t 1 if succesful, S_SERIAL_ERR if error.
     */
    uint32_t append_crc16(uint8_t* const buf, const std::size_t cap, const uint16_t offset, const uint16_t crc);

    uint32_t append_crc16(Frame::Raw_frame* const frame, const std::size_t cap, const uint16_t offset, const uint16_t crc);

}} // namespace CRC, Smart_serial

#endif // SMART_SERIAL_CRC_HPP