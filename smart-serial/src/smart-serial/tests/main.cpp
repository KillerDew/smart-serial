#include "smart-serial/port/mock_port.hpp"
#include <iostream>

int main() {

    Smart_serial::Mock_port<> mock_port;

    uint8_t buf[] = {
        0x00,
        0x01,
        0x02,
        0x03,
        0x04
    };
    mock_port.write(buf, sizeof(buf));
    mock_port.inject_rx(buf, sizeof(buf), false);

    std::cout << "rx data: ";
    for (size_t i = 0; i < 512; ++i) {
        int32_t byte = mock_port.read_byte();
        if (byte != S_SERIAL_ERR) {
            std::cout << std::hex << byte << " ";
        } else {
            break;
        }
    }

    return 0;
}