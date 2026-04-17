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

#include "clock/IClock.hpp"
#include "frame.hpp"
#include "port/iport.hpp"
#include "error.h"
#include <cstdint>

namespace Smart_serial {

    enum Receive_result {
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
            * @brief Construct a new Master object with port and a default timout
            * 
            * @param slave_addr The slave address byte
            * @param port An IPort reference (PROS wrapper or mock port)
            * @param default_timeout The default timeout to use when receiving/transacting
            */
            Master(I_port& port,
                   Clock::I_clock& clock_,
                   const uint8_t slave_addr,
                   const uint32_t default_timeout);

            /** @brief Trivial constructor, no dynamic mem allocation */
            ~Master() {};

            /**
             * @brief Receive a packet over the smart port
             * 
             * @param packet_out The packet result from receive
             * @param timeout The timeout to use. Defaults to 5 seconds
             * @param check_crc Whether to check the crc or not
             * @return Receive_result, success (1) or appropriate error result 
             */
            Receive_result receive_packet(Packet_result* const packet_out,
                                    const uint32_t timeout,
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
             * @return Receive_result result of the transaction
             */
            Receive_result transact(Packet_result* const packet_out,
                              const Frame::Frame* const frame_in,
                              const uint32_t timeout);

            /**
             * @brief Handshake with the slave
             * 
             * @param timeout The timeout to use. Defaults to 5 seconds
             * @return Receive_result The result of the transaction
             */
            Receive_result handshake(const uint32_t timeout);
            
            /** @brief set start byte for transmissions and receives
                @param byte byte to use */
            void set_start_byte(const uint8_t byte) { start_byte = byte; };
        private:
            I_port& serial_port;
            uint8_t slave_address;

            uint8_t start_byte = 0xAA;

            const Clock::I_clock& clock;

            const uint32_t DEFAULT_TIMEOUT;

            /**
             * @brief Reads a raw frame from the ports buffer
             * 
             * @param raw_frame_out The raw frame (data, data len)
             * @param timeout The timeout to use, defaults to 3s
             * @return uint32_t 1 if successful, S_SERIAL_ERR if not
             */
            uint32_t read_raw_frame(Frame::Raw_frame* const raw_frame_out, const uint32_t timeout=0);

            Master(Master&) = delete;
            Master operator=(Master&) = delete;

    };
}

#endif // SMART_SERIAL_MASTER_HPP