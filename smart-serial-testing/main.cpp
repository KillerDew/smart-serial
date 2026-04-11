#include "./include/smart-serial/port/mock_port.hpp"
#include <iostream>
#include <conio.h>
#include <iomanip>

int main() {
    Smart_serial::Mock_port<256U, 256U> mock_port;

    uint8_t buf[] = {
        0x00,
        0x01,
        0x02,
        0x03,
        0x04,
        0x06,
        0x09,
        0x0A,
        0x08,
    };
    int32_t res = mock_port.write(buf, sizeof(buf));
    std::cout << "Bytes written: " << res << std::endl;
    mock_port.inject_rx(buf, sizeof(buf), false);

    std::cout << "tx data: " << std::endl;
    const uint8_t* myBytes = mock_port.get_tx_data();
    for (size_t j = 0; j < 256U; ++j) {
        std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') << (int)myBytes[j] << " ";
    }
    getch();
    return 0;
}