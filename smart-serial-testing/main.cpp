#include "./include/smart-serial/clock/Std_clock.hpp"
#include "./include/smart-serial/port/mock_port.hpp"
#include "./include/smart-serial/master.hpp"
#include "smart-serial/crc.hpp"
#include <cstdint>
#include <cstdio>
#include <iostream>
using namespace std;
int main() {
    Smart_serial::Clock::Std_clock clock;
    Smart_serial::Mock_port<256U, 256U> mock_port;
    Smart_serial::Master master (
        mock_port,
        clock,
        0xAA,
        1000U
    );

    uint8_t fake_rx[7U] = {
        0xaa, 0xaa, 0x01, 0x16, 0x00
    };
    uint16_t crc = Smart_serial::CRC::compute_crc16(fake_rx, 5U);
    Smart_serial::CRC::append_crc16(fake_rx, 256U, 5U, crc);
    Smart_serial::Receive_result res = master.handshake(3U, 1000U);
    printf("%i", res);
    
    
    
    return 0;
}