#include "./include/smart-serial/port/mock_port.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <conio.h>
#include <iomanip>
#include <ostream>
#include "include/smart-serial/error.h"
#include "smart-serial/crc.hpp"
#include "smart-serial/frame.hpp"
using namespace std;
int main() {
    
    const uint8_t* payload = reinterpret_cast<const uint8_t*>("Hello World!");
    size_t len = strlen("Hello World!");

    Smart_serial::Frame::Frame frame;
    uint32_t result = Smart_serial::Frame::build_frame(&frame, 0x02, 0xAA, 0xFF, 0x00, payload, len);
    if (result != S_SERIAL_ERR) {
        Smart_serial::Frame::Raw_frame r_frame;
        uint32_t res = Smart_serial::Frame::dump_frame(&r_frame, &frame);
        uint16_t crc = Smart_serial::CRC::compute_crc16(r_frame.data, r_frame.length);
        res = Smart_serial::CRC::append_crc16(&r_frame, 256U, 0U, crc);
        if (res != S_SERIAL_ERR) {
            for (size_t i=0; i<5U; ++i) {
                printf("%02X ", r_frame.data[i]);
            }
            for (size_t i=5; i<r_frame.length-2; ++i) {
                std::cout << r_frame.data[i];
            }
            printf(" %02X %02X\n", r_frame.data[r_frame.length-2], r_frame.data[r_frame.length-1]);
            std::cout << endl;

            Smart_serial::Frame::Frame reconstructed_frame;
            uint32_t res_ = Smart_serial::Frame::parse_frame(&reconstructed_frame, r_frame.data, 0U, r_frame.length);
            if (res_ != S_SERIAL_ERR) {
                printf("Payload length: %i\n", reconstructed_frame.header.payload_len);
                printf("Payload: ");
                for (int i=0; i<reconstructed_frame.header.payload_len; ++i) {cout << reconstructed_frame.payload[i];}
            }

        } else {
            std::cout << "Error dumping frame / Computing CRC" << endl;
        }
    } else {
        std::cout << "Error building frame" << endl;
    }

    
    return 0;
}