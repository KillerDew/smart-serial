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
#include "error.h"
#include <cstdint>

using namespace Smart_serial;
namespace Smart_serial {

    enum Recieve_result {
        SUCCESS = 1,
        ERR_NACK = 0,
        ERR_CRC = -1,
        ERR_TIMEOUT = -2,
        ERR_PROCESS = S_SERIAL_ERR
    };

    struct Packet_result {
        Frame::Frame frame;
        uint16_t CRC;
        bool passed_CRC;
    };

    class Master {
        public:
            /**
            * @brief Construct a new Master object with port and a defualt timout
            * 
            * @param slave_addr The slave address byte
            * @param port An IPort reference (PROS wrapper or mock port)
            * @param defualt_timeout The default timeout to use when recieving/transacting
            */
            Master(const I_port& port,
                   const uint8_t slave_addr,
                   const uint32_t defualt_timeout);

            /** @brief Trivial constructor, no dynamic mem allocation */
            ~Master();

            /**
             * @brief Recieve a packet over the smart port
             * 
             * @param packet_out The packet result from recieve
             * @param timeout The timeout to use. Defaults to 5 seconds
             * @param check_crc Whether to check the crc or not
             * @return Recieve_result, success (1) or appropriate error result 
             */
            Recieve_result recieve_packet(Packet_result* const packet_out,
                                    uint32_t timeout,
                                    bool check_crc);

            /**
             * @brief Send a frame
             * 
             * @param frame The frame to send
             * @param timeout the timeout to use. Defaults to 5 seconds
             * @return uint32_t 1 if successful, S_SERIAL_ERR if not
             */
            uint32_t send_frame(const Frame::Frame* const frame,
                                const uint32_t timeout);
            /**
             * @brief 
             * 
             * @param packet_out The packet response from slave
             * @param frame_in The frame to send
             * @param timeout the timeout to use. Defaults to 5 seconds
             * @return Recieve_result result of the transaction
             */
            Recieve_result transact(Packet_result* const packet_out,
                              const Frame::Frame* const frame_in,
                              const uint32_t timeout);

            /**
             * @brief Handshake with the slave
             * 
             * @param timeout The timeout to use. Defaults to 5 seconds
             * @return Recieve_result The result of the transaction
             */
            Recieve_result handshake(uint32_t timeout);
        private:
            const I_port& serial_port;
            const uint8_t slave_address;

            const uint32_t DEFUALT_TIMEOUT;

            /**
             * @brief Reads a raw frame from the ports buffer
             * 
             * @param raw_frame_out The raw frame (data, data len)
             * @param timeout The timeout to use, defaults to 3s
             * @return uint32_t 1 if successful, S_SERIAL_ERR if not
             */
            uint32_t read_raw_frame(Frame::Raw_frame* const raw_frame_out, uint32_t timeout);

            Master(Master&) = delete;
            Master operator=(Master&) = delete;

    };
}

#endif // SMART_SERIAL_MASTER_HPP