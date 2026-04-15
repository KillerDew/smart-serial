/**
 * @file Pros_clock.hpp
 * @author Daniel Dew
 * @brief Wrapper for PROS clock functions. Implements virtual I_clock
 * @version 0.1
 * @date 2026-04-15
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#ifndef SMART_SERIAL_PROS_CLOCK_HPP
#define SMART_SERIAL_PROS_CLOCK_HPP

#include "IClock.hpp"
#include "pros/rtos.hpp"
#include <cstdint>

namespace Smart_serial { namespace Clock {

    class Pros_clock : public I_clock {
        public:
            /**
             * @brief Get the current run time. Delegates to pros::millis()
             * @return uint32_t The time in milliseconds
             */
            uint32_t millis() const override { return pros::millis(); }
            /**
             * @brief Delay program execution. Delegates to pros::delay()
             * @param delay_time Time in milliseconds to delay program
             */
            void delay(uint32_t delay_time) const override { pros::delay(delay_time); }

    };
}} // Namespaces

#endif // SMART_SERIAL_PROS_CLOCK_HPP