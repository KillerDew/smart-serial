/**
 * @file Std_clock.hpp
 * @author Daniel Dew
 * @brief Contains definitions (and implementations) for std clock wrappers to use when building from desktop
 * @version 0.1
 * @date 2026-04-15
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef SMART_SERIAL_STD_CLOCK_HPP
#define SMART_SERIAL_STD_CLOCK_HPP

#include "IClock.hpp"
#include <chrono>
#include <cstdint>
#include <thread>

namespace Smart_serial { namespace Clock {

    class Std_clock : public I_clock {
        public:
            uint32_t millis() const override {
                using namespace std::chrono;
                return static_cast<uint32_t>(
                    duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count()
                );
            };
            void delay(uint32_t delay_time) const override {
                std::this_thread::sleep_for(std::chrono::milliseconds(delay_time));
            }
    };

}} // namespaces

#endif // SMART_SERIAL_STD_CLOCK_HPP