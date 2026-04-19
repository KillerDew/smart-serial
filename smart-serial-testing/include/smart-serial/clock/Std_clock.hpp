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
            /** @brief Construct the clock and initialise its start time */
            Std_clock() : start_time(std::chrono::steady_clock::now()) {}
            /**
             * @brief Get elapsed time from clock initialisation
             * 
             * @return uint32_t Time in milliseconds
             */
            uint32_t millis() const override {
                using namespace std::chrono;
                return static_cast<uint32_t>(
                    duration_cast<milliseconds>(steady_clock::now() - start_time).count()
                );
            };
            /**
             * @brief Delay the program
             * 
             * @param delay_time time in milliseconds to delay
             */
            void delay(uint32_t delay_time) const override {
                std::this_thread::sleep_for(std::chrono::milliseconds(delay_time));
            }
            ~Std_clock() {};
        private:
            std::chrono::steady_clock::time_point start_time;
    };

}} // namespaces

#endif // SMART_SERIAL_STD_CLOCK_HPP