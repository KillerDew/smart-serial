/**
 * @file crc.cpp
 * @author Daniel Dew
 * @brief Contains implementation for CRC related utilities.
 * @version 0.1
 * @date 2026-04-11
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "smart-serial/crc.hpp"
#include "smart-serial/error.h"
#include <cstddef>
#include <cstdint>

using namespace Smart_serial;

uint16_t CRC::compute_crc16(const uint8_t* const data, uint16_t length) {
    uint16_t result = 0;
    if (data != NULL){
        // Constants for CRC calculation
        const uint8_t BITS = 8U;
        const uint16_t POLY = 0x1021U; // CRC-CCITT polynomial
        const uint16_t MSB_MASK = 0x8000U; // Mask to check the most significant bit (0b1000000000000000)

        // Initialize CRC to 0xFFFF
        uint16_t crc = 0xFFFF;
        // Pointer to current byte being processed (inits to first)
        const uint8_t* current_byte_ptr = data;

        for (size_t byte_idx=0; byte_idx<length; ++byte_idx) {
            // XOR the current byte (shifted to the upper byte) with the current CRC value
            crc ^= static_cast<uint16_t>(static_cast<uint16_t>(*current_byte_ptr) << 8U);

            ++current_byte_ptr; // Move pointer to next byte
            
            // If MSB is 1, shift left and XOR with polynomial, otherwise just shift left. Repeat for each bit.
            for (uint8_t bit_idx=0; bit_idx<BITS; ++bit_idx) {
                if ((crc & MSB_MASK) != 0U) {
                    crc = static_cast<uint16_t>((crc << 1U) ^ POLY);
                } else {
                    crc <<= 1U;
                }
            }
        }
        result = crc;
    } else {
        result = S_SERIAL_ERR_2_BYTE;
    }
    return result;
}

uint16_t CRC::extract_crc16(const uint8_t* const data, uint16_t offset) {
    uint16_t result = 0;
    if (data != NULL){
        // Extract little endian from data
        uint8_t low_byte = data[offset];
        uint8_t high_byte = data[offset+1U];

        // reconstruct CRC from high byte and low byte (high byte shifted to top, ORed with low byte)
        result = static_cast<uint16_t>((high_byte << 8U) | low_byte);
    } else result = S_SERIAL_ERR_2_BYTE;

    return result;
}

uint32_t CRC::append_crc16(uint8_t* const  buf,
                             const size_t    cap,
                             const uint16_t  offset,
                             const uint16_t  crc)
{
    uint32_t result = S_SERIAL_ERR;
    // Check if buffer is null and that there is space for CRC
    if ((buf != NULL) &&
        ((static_cast<size_t>(offset) + 2U) <= cap))
    {
        // Mask that isolates the lower byte
        static const uint16_t ONE_BYTE_MASK = 0x00FFU;
        // Seperates the low and high bytes from CRC
        const uint8_t low_byte  = static_cast<uint8_t>(crc & ONE_BYTE_MASK);
        const uint8_t high_byte = static_cast<uint8_t>((crc >> 8U) & ONE_BYTE_MASK);
        // Appends CRC bytes using little endian (low byte first)
        buf[offset]      = low_byte;
        buf[offset + 1U] = high_byte;

        result = 1U; // Successful
    }
    
    return result;
}