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

    enum Default_cmds {
        ACK = 0x06,
        NACK = 0x15
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
            Receive_result receive_packet(Frame::Frame* const frame_out,
                                    const uint32_t timeout,
                                    bool check_crc=true);
            /**
             * @brief Sends a string alongside a command
             * 
             * @param str String to send
             * @param cmd_byte Command byte to send
             * @return uint32_t 1 if successful, S_SERIAL_ERR if not.
             */
            uint32_t send_string(const char* const str, const uint8_t cmd_byte);
            /**
             * @brief Sends byte buffer alongside a command
             * 
             * @param data Byte buffer to send
             * @param cmd_byte command to send
             * @return uint32_t 1 if successful, S_SERIAL_ERR if not.
             */
            uint32_t send_bytes(const uint8_t* const data, const uint8_t cmd_byte);

            /**
             * @brief 
             * 
             * @param packet_out The packet response from slave
             * @param frame_in The frame to send
             * @param timeout the timeout to use. Defaults to 5 seconds
             * @return Receive_result result of the transaction
             */
            Receive_result transact(Frame::Frame* const frame_out,
                              const Frame::Frame* const frame_in,
                              const uint32_t timeout=0U);
            Receive_result transact(Frame::Frame* const frame_out,
                              const uint8_t* const buf,
                              const uint8_t cmd_byte,
                              const uint32_t timeout=0U);
            Receive_result transact(Frame::Frame* const frame_out,
                              const char* const str,
                              const uint8_t cmd_byte,
                              const uint32_t timeout=0U);

            /**
             * @brief Handshake with the slave
             * 
             * @param timeout The timeout to use. Defaults to 5 seconds
             * @return Receive_result The result of the transaction
             */
            Receive_result handshake(const uint32_t attempts, const uint32_t timeout_per=0U);
            
            /** @brief set start byte for transmissions and receives
                @param byte byte to use */
            void set_start_byte(const uint8_t byte) { start_byte = byte; };
            /**
             * @brief Set the master address value
             * @param byte byte to set to */
            void set_master_address(const uint8_t byte) { this_address = byte; }
        private:
            I_port& serial_port;
            uint8_t slave_address;
            
            uint8_t start_byte = 0xAA;
            uint8_t this_address = 0x01;

            const Clock::I_clock& clock;

            const uint32_t DEFAULT_TIMEOUT;
                
            /**
            * @brief Send a frame
            * 
            * @param frame The frame to send
            * @param timeout the timeout to use. Defaults to 5 seconds
            * @return int32_t 1 if successful, S_SERIAL_ERR if not
            */
            uint32_t send_frame(const Frame::Frame* const frame);

            /**
             * @brief Reads a raw frame from the ports buffer
             * 
             * @param raw_frame_out The raw frame (data, data len)
             * @param timeout The timeout to use, defaults to 3s
             * @return uint32_t 1 if successful, S_SERIAL_ERR if not
             */
            uint32_t read_raw_frame(Frame::Raw_frame* const raw_frame_out, const uint32_t timeout=0U);

            Master(Master&) = delete;
            Master operator=(Master&) = delete;

    };
}

#endif // SMART_SERIAL_MASTER_HPP