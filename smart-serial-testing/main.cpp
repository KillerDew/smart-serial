#include "./include/smart-serial/port/mock_port.hpp"
#include <iostream>
#include <conio.h>
#include <iomanip>
#include "smart-serial/crc.hpp"

int main() {
    const uint8_t buf[2] {
        0xaa, 0xbb
    };
    std::cout << Smart_serial::CRC::compute_crc16(buf, 2) << std::endl;
    getch();
    return 0;
}