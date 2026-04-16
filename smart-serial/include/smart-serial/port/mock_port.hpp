/**
 * @file mock_port.hpp
 * @author Daniel Dew
 * @brief Contains definitions and implementations for a mock port class
 * Used for testing withouut access to VEX hardware. Implements the I_port interface.
 * @version 0.1
 * @date 2026-04-10
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#ifndef MOCK_PORT_HPP
#define MOCK_PORT_HPP

#include "smart-serial/port/iport.hpp"
#include "smart-serial/error.h"
#include <cstdint>
#include <cstring>

namespace Smart_serial {

    /**
     * @brief Template class for mock serial port. Implements IPort.
     * 
     * @tparam TX_CAP TX buffer capacity. Default 256 bytes.
     * @tparam RX_CAP RX buffer capacity. Default 256 bytes.
     */
    template<uint16_t TX_CAP = 256U, uint16_t RX_CAP = 256U>
    class Mock_port : public I_port {
        public:
            /**
             * @brief Initialise buffers and lengths.
             * 
             */
            Mock_port();
            ~Mock_port(); // Trivial destructor, no dynamic memory.

            // Overrides from I_port
            
            /**
             * @brief Write data to the mock port.
             * 
             * @param buf buffer to write from. Must not exceed TX_CAP.
             * @param len length of data to write. Must not exceed TX_CAP.
             * @return int32_t Number of bytes written, or S_SERIAL_ERR on error
             */
            int32_t write (uint8_t *buf, uint16_t len) override;

            /**
             * @brief Read single byte from mock rx buffer. Must be injected into first using inject_rx().
             * 
             * @return int32_t 
             */
            uint8_t read_byte() override;

            /**
             * @brief Flushes rx buffer.
             */
            void flush_rx() override;

            /**
             * @brief Inject data into rx buffer.
             * 
             * @param buf buffer to inject from. Must not exceed RX_CAP.
             * @param len length of data to inject. Must not exceed RX_CAP.
             * @param append if true, appends to existing rx buffer. If false, overwrites it. Default false.
             * @return int32_t Number of bytes injected, or S_SERIAL_ERR on error.
             */
            int32_t inject_rx(const uint8_t *buf, uint16_t len, bool append);

            /**
             * @brief Get the tx data from buffer
             * 
             * @return const uint8_t* buffer containing tx data.
             */
            const uint8_t* get_tx_data() const;

            /**
             * @brief Get the tx buffer length
             * 
             * @return const uint16_t number of bytes in tx buffer.
             */
            uint16_t get_tx_data_len() const;

            /**
             * @brief Reset port and all buffers.
             */
            void reset();

        private:
            uint8_t tx_buf[TX_CAP];
            uint16_t tx_len;

            uint8_t rx_buf[RX_CAP];
            uint16_t rx_len;
            uint16_t rx_read_head;

            Mock_port(const Mock_port&) = delete; // Non-copyable
            Mock_port& operator=(const Mock_port&) = delete; // Non-assignable
    };

    // Template definitions. Must be in header. 

    // Initialise buffers and lengths.
    template<uint16_t TX_CAP, uint16_t RX_CAP>
    Mock_port<TX_CAP, RX_CAP>::Mock_port() : tx_len(0), rx_len(0), rx_read_head(0) {
        static_cast<void>(std::memset(tx_buf, 0, sizeof(tx_buf)));
        static_cast<void>(std::memset(rx_buf, 0, sizeof(rx_buf)));
    }

    template<uint16_t TX_CAP, uint16_t RX_CAP>
    Mock_port<TX_CAP, RX_CAP>::~Mock_port() {} // No dynamic memory, trivial destructor.

    template<uint16_t TX_CAP, uint16_t RX_CAP>
    int32_t Mock_port<TX_CAP, RX_CAP>::write(uint8_t *buf, uint16_t len) {
        int32_t result = S_SERIAL_ERR;
        if ((buf != NULL) && (tx_len <= (TX_CAP - len))) { // Check for null buffer and sufficient space in tx buffer.
            static_cast<void>(
                std::memcpy( &tx_buf[tx_len], buf, static_cast<size_t>(len) )
            );
            tx_len = static_cast<uint16_t>(tx_len + len);
            result = static_cast<int32_t>(len);
        }
        return result;
    }

    template<uint16_t TX_CAP, uint16_t RX_CAP>
    uint8_t Mock_port<TX_CAP, RX_CAP>::read_byte() {
        uint8_t result = S_SERIAL_ERR_BYTE;
        if (rx_read_head < rx_len) {
            result = static_cast<uint8_t>(rx_buf[rx_read_head]);
            ++rx_read_head;
        }
        return result;
    }

    // Moves read head to end of buffer, effectively flushing it while preserving data.
    template<uint16_t TX_CAP, uint16_t RX_CAP>
    void Mock_port<TX_CAP, RX_CAP>::flush_rx() {
        rx_read_head = rx_len;
    }

    // Injects data into rx buffer, either appending or overwriting. Returns number of bytes injected.
    template<uint16_t TX_CAP, uint16_t RX_CAP>
    int32_t Mock_port<TX_CAP, RX_CAP>::inject_rx(const uint8_t *buf, uint16_t len, bool append) {
        int32_t result = S_SERIAL_ERR;
        // Check for sufficient space and if buf is NULL.
        if (append && (buf != NULL)) {
            if (rx_len <= (RX_CAP - len)) {
                static_cast<void>(
                    // Offset write to end of existing data.
                    std::memcpy( &rx_buf[rx_len], buf, static_cast<size_t>(len) )
                );
                rx_len = static_cast<uint16_t>(rx_len + len);
                result = static_cast<int32_t>(len);
            }
        } else {
            if (len <= RX_CAP) {
                static_cast<void>(
                    // Rewrite from start of buffer.
                    std::memcpy(rx_buf, buf, static_cast<size_t>(len))
                );
                // Set length of rx buffer to length of written data
                rx_len = len;
                rx_read_head = 0U; // Reset read head to start of buffer.
                result = static_cast<int32_t>(len);
            }
        }
        return result;
    }
    // Returns pointer to tx buffer. Const to prevent modification of internal buffer.
    template<uint16_t TX_CAP, uint16_t RX_CAP>
    const uint8_t* Mock_port<TX_CAP, RX_CAP>::get_tx_data() const {
        return tx_buf;
    }
    // Returns number of bytes in tx buffer.
    template<uint16_t TX_CAP, uint16_t RX_CAP>
    uint16_t Mock_port<TX_CAP, RX_CAP>::get_tx_data_len() const {
        return tx_len;
    }

    // Resets buffers and lengths to initial state.
    template <uint16_t TX_CAP, uint16_t RX_CAP>
    void Mock_port<TX_CAP, RX_CAP>::reset() {
        tx_len = 0U;
        rx_len = 0U;
        rx_read_head = 0U;
        static_cast<void>(std::memset(tx_buf, 0, sizeof(tx_buf)));
        static_cast<void>(std::memset(rx_buf, 0, sizeof(rx_buf)));
    }


} // namespace Smart_serial

#endif // MOCK_PORT_HPP