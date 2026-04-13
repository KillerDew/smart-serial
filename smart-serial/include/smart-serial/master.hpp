/**
 * @file master.hpp
 * @author Daniel Dew
 * @brief Contains definitions for a serial Master device
 * @version 0.1
 * @date 2026-04-13
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef SMART_SERIAL_MASTER_HPP
#define SMART_SERIAL_MASTER_HPP

#include "frame.hpp"
#include "port/iport.hpp"
#include <cstdint>

using namespace Smart_serial;
namespace Smart_serial {

    struct Packet_result {
        Frame::Frame frame;
        uint16_t CRC;
        bool passed_CRC;
    };

    class Master {
        public:
            uint32_t recieve_packet(Packet_result* const packet_out, uint32_t timeout, bool check_crc);
            uint32_t send_payload(const uint8_t* payload_bytes, const uint8_t cmd);
            uint32_t send_payload(const char* const payload_str, const uint8_t cmd);
            uint32_t transact(Packet_result* const packet_out, const uint8_t cmd, const char* const payload_str, const uint32_t timeout);
            uint32_t transact(char* const payload_str_out, const uint8_t cmd, const char* const payload_str, const uint32_t timeout);
            uint32_t transact(uint8_t* const payload_out, const uint8_t cmd, const char* const payload_str, const uint32_t timeout);
            uint32_t handshake(uint32_t timeout);
        private:
            I_port& serial_port;
            uint8_t slave_address;

            uint32_t read_raw_frame(Frame::Raw_frame* const raw_frame_out, uint32_t timeout);

    };
}

#endif // SMART_SERIAL_MASTER_HPP