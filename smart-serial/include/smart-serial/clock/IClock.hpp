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
        
            virtual ~I_clock();
            virtual uint32_t millis() const = 0;
            virtual uint32_t delay(uint32_t delay_time) const = 0;
            virtual uint32_t delayed_call(void (*func)(), uint32_t delay);
        private:

            I_clock(const I_clock&) = delete;
            I_clock& operator=(const I_clock&) = delete;
    };

}}

#endif // SMART_SERIAL_ICLOCK_HPP