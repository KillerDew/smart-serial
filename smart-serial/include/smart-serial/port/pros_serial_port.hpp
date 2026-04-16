/**
 * @file pros_serial_port.hpp
 * @author Daniel Dew
 * @brief Wraps PROS serial port functionality in a class that implements the I_port interface.
 * @version 0.1
 * @date 2026-04-09
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef SMART_SERIAL_PROS_SERIAL_PORT_HPP
#define SMART_SERIAL_PROS_SERIAL_PORT_HPP

#include "smart-serial/port/iport.hpp"
#include "pros/serial.hpp"
#include <cstdint>


#define SMART_SERIAL_DEFAULT_BAUD_RATE (115200)

namespace Smart_serial {
    
    class Pros_serial_port : public I_port {
        public:
            /**
             * @brief Construct and open a PROS serial port.
             * 
             * @param smart_port_num V5 Port Number (1-21)
             * @param baud_rate Buad rate bits/second. Default is 115200.
             */
            explicit Pros_serial_port(const uint32_t smart_port_num, const uint32_t baud_rate);

            /** @brief destructor - trivial as PROS cleans up itself */
            ~Pros_serial_port();

            /**
             * @brief Transmit bytes over the serial port.
             * 
             * @param buf buffer of bytes to transmit. Must not be NULL.
             * @param len number of bytes to transmit
             * @return int32_t Bytes successfully transmitted, or S_SERIAL_ERR on error
             */
            int32_t write(uint8_t *buf, uint16_t len) override;

            /**
             * @brief Read a single byte from the serial port without blocking.
             * 
             * Delegates to pros::Serial::read_byte().
             *
             * @return int32_t Byte value read, or S_SERIAL_ERR if no byte is available
             */
            uint8_t read_byte() override;

            /**
             * @brief flush serial recieve buffer
             * 
             * uses read_byte() in loop until empty, BOUNDED <AV Rule 113>
             */
            void flush_rx() override;
        
        private:
            /** @brief maximum iteratons to flush RX buffer <AV Rule 113> */
            static const uint16_t MAX_FLUSH_ITERS = 512U;

            /** @brief underlying pros serial port */
            pros::Serial serial_port;

            /** AV Rule 88 */
            Pros_serial_port(const Pros_serial_port&);
            Pros_serial_port& operator=(const Pros_serial_port&);

    };
}


#endif // SMART_SERIAL_PROS_SERIAL_PORT_HPP