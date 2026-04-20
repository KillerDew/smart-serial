#include "tests/tests.hpp"
#include "smart-serial/frame.hpp"
#include "smart-serial/crc.hpp"
#include "smart-serial/port/mock_port.hpp"
#include "smart-serial/clock/Std_clock.hpp"
#include "smart-serial/slave.hpp"
#include <cstdio>

using namespace Smart_serial;
using namespace Smart_serial::CRC;
using namespace Smart_serial::Clock;

int Tests::test_slave() {

    printf("-- Testing slave device implementations --\n");
    Mock_port<256, 256> port;
    Std_clock clock;
    Slave slave(port, clock, 0x02U, 1000U);
    int pass=0; int fail = 0;
    // Helper lambda: builds a frame and injects it into the port's rx buffer
    auto inject_frame = [&](uint8_t cmd, const uint8_t* payload, size_t payload_len) {
        Frame::Frame f{};
        build_frame(&f, 0xAAU, 0x02U, 0x01U, cmd, payload, payload_len);
        Frame::Raw_frame raw{};
        dump_frame(&raw, &f);
        // Append CRC
        uint16_t crc = compute_crc16(raw.data, raw.length);
        append_crc16(&raw, Frame::S_SERIAL_MAX_FRAME_BYTES, raw.length, crc);
        port.inject_rx(raw.data, static_cast<uint16_t>(raw.length), false);
    };
    
    

    printf("%i/%i passed", pass, pass+fail);

    return (fail == 0);
}