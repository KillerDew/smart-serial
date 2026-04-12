#include "./include/smart-serial/port/mock_port.hpp"
#include <iostream>
#include <conio.h>
#include <iomanip>
#include <ostream>
#include "smart-serial/crc.hpp"
using namespace std;
int main() {
    uint8_t buf[6] {
        0x12, 0x13, 0xaa, 0xbb, 0x00, 0x00
    };

    uint16_t crc = Smart_serial::CRC::compute_crc16(buf, 4);
    printf("%02X \n", crc);
    uint32_t result = Smart_serial::CRC::append_crc16(buf, 6U, 4, crc);
    std::cout << result << std::endl;
    for (int i=0; i<6; ++i) {
        printf("%02X ", buf[i]);
    }
    
    getch();
    return 0;
}