/**
 * @file IClock.hpp
 * @author Daniel Dew
 * @brief Contains definitions for a generic clock interface
 * @version 0.1
 * @date 2026-04-15
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef SMART_SERIAL_ICLOCK_HPP
#define SMART_SERIAL_ICLOCK_HPP

#include <cstdint>

namespace Smart_serial { namespace Clock {

    class I_clock {
        public:
            /** @brief Trivial constructor */
            virtual ~I_clock() {}
            /**
             * @brief Get the current run time
             * @return uint32_t The time in milliseconds
             */
            virtual uint32_t millis() const = 0;
            /**
             * @brief Delay program execution
             * @param delay_time Time in milliseconds to delay program
             */
            virtual void delay(uint32_t delay_time) const = 0;
        private:
            I_clock(const I_clock&) = delete;
            I_clock& operator=(const I_clock&) = delete;
    };

}} // namespaces

#endif // SMART_SERIAL_ICLOCK_HPP