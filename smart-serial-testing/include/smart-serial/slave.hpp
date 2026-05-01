/**
 * @file slave.hpp
 * @author Daniel Dew
 * @brief Contains defintions for the slave device
 * @version 0.1
 * @date 2026-04-19
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef S_SERIAL_SLAVE_HPP
#define S_SERIAL_SLAVE_HPP

#include "types.h"
#include "port/iport.hpp"
#include "clock/IClock.hpp"
#include "frame.hpp"
#include <cstdint>

namespace Smart_serial {

    class Slave {
        public:
            /**
             * @brief Construct a new Slave device
             * 
             * @param port Serial (interface) port to read and write from
             * @param clock The clock (interface) to use
             * @param this_addr the address of this device
             * @param master_addr The address of the master device
             * @param default_timeout Default timeout to use.
             */
            Slave(
                I_port& port, 
                Clock::I_clock& clock, 
                uint8_t this_addr,
                uint8_t master_addr,
                uint8_t start_byte,
                uint32_t default_timeout
            );
            /** @brief Destroy the Slave object */
            ~Slave();

            /**
             * @brief Recieve a request (frame)
             * 
             * @param frame_out The frame that is read
             * @param timeout The timeout to use (0 = device default)
             * @return Receive_result The status of the receive operation
             */
            Receive_result receive_request(Frame::Frame* const frame_out, uint32_t timeout);

            /**
             * @brief Send a response using a buffer as the payload
             * 
             * @param buf The payload to send
             * @param length Length of byte buffer
             * @param cmd_byte The command to send with payload
             * @return int32_t 1 if successful, corresponding error code if not
             */
            int32_t send_response(const uint8_t* const buf,
                                  const size_t length,
                                  const uint8_t cmd_byte);

            /**
             * @brief Send a response using a string as a payload
             * 
             * @param str String to send
             * @param cmd_byte The command to send with payload
             * @return int32_t 1 if successful, corresponding error code if not
             */
            int32_t send_response(const char* const str, const uint8_t cmd_byte);

            /** @brief set the start byte for a transmission (and for reading)
                @param start_byte byte to use*/
            void set_start_byte(const uint8_t start_byte);
            /** @brief set this device's address
                @param adress address byte */
            void set_this_address(const uint8_t address);
            /** @brief set the master's adress
                @param address the address byte */
            void set_master_adress(const uint8_t address);
            /** @brief set whether or not to automatically ACK on a received handshake packet
                @param auto_shake value to set to */
            void set_auto_handshake(const bool auto_shake);
        private:
            I_port& serial_port;
            
            uint8_t this_address;

            uint8_t master_address;

            uint8_t start_byte;

            bool auto_handshake;

            const Clock::I_clock& clock;

            const uint32_t DEFAULT_TIMEOUT;

            /**
             * @brief Send a response using a built frame
             * 
             * @param frame built frame to send
             * @return int32_t 1 if successful, corresponding error code if not
             */
            int32_t send_response(const Frame::Frame* frame);

            /**
             * @brief Read a raw frame from serial buffer
             * 
             * @param frame_out Raw frame output
             * @param timeout timeout to use (0 = device default)
             * @return int32_t 1 if successful, corresponding error code if not
             */
            int32_t read_raw_frame(Frame::Raw_frame* const raw_frame_out, uint32_t timeout=0U);

            /**
             * @brief validate the crc of a raw frame
             * 
             * @param raw_frame Raw frame to check
             * @return int32_t ERR_CRC if crc failed, 1 if check passed; or corresponding error code
             */
            int32_t validate_crc(const Frame::Raw_frame* raw_frame);

            Slave(Slave&) = delete;
            Slave& operator=(Slave&) = delete;
            
    };
    
};

#endif // S_SERIAL_SLAVE_HPP