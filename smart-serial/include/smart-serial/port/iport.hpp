/**
 * @file iport.hpp 
 * @author Daniel Dew
 * @brief Abstract serial port interface
 * @version 0.1
 * @date 2026-04-07
 * 
 * @copyright Copyright (c) 2026
 *
 * @standard JSF AV C++ Coding Standard (2005)
 * 
 */
#ifndef SMART_SERIAL_IPORT_HPP
#define SMART_SERIAL_IPORT_HPP

#include <cstdint>

namespace Smart_serial {

    /**
     * @brief Virtual interface for serial ports. Abstracts away serial ports for standardisation across hardware.
     * 
     */
     class I_port {

        public:
            /**
            * @brief Destroy the i port object
            */
            virtual ~I_port() {};
            /**
             * @brief writes data to the serial buffer and transmits
             * 
             * @param data pointer to bytes to write. Must not be NULL.
             * @param len length of data to be transmitted
             * @return int32_t number of bytes successfully transmitted, or S_SERIAL_ERR on error
             */
            virtual int32_t write(uint8_t const *buf, uint16_t len) = 0;
        
            /**
             * @brief reads single byte from serial buffer without blocking.
             * 
             * @return int32_t the byte read, or S_SERIAL_ERR if no data is available
             */
            virtual int32_t read_byte() = 0;

            /**
             * @brief flushes the recieve buffer, discarding all data in it.
             */
            virtual void flush_rx() = 0;
        

    };
}

#endif // SMART_SERIAL_IPORT_HPP