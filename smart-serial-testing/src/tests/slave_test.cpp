#include "smart-serial/master.hpp"
#include "smart-serial/types.h"
#include "tests/tests.hpp"
#include "smart-serial/frame.hpp"
#include "smart-serial/crc.hpp"
#include "smart-serial/port/mock_port.hpp"
#include "smart-serial/clock/Std_clock.hpp"
#include "smart-serial/slave.hpp"
#include <cstdint>
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
    auto inject_frame = [&](uint8_t cmd, const uint8_t* payload, size_t payload_len, uint8_t to_addr=0x02U, uint16_t crc=0U) {
        Frame::Frame f{};
        build_frame(&f, 0xAAU, 0xFFU, to_addr, cmd, payload, payload_len);
        Frame::Raw_frame raw{};
        dump_frame(&raw, &f);
        // Append CRC
        if (crc==0) crc = compute_crc16(raw.data, raw.length);
        append_crc16(&raw, Frame::S_SERIAL_MAX_FRAME_BYTES, raw.length, crc);
        port.inject_rx(raw.data, static_cast<uint16_t>(raw.length), false);
    };

    {
        // TEST 1: RECIEVING ACK (handle)
        slave.set_auto_handshake(true);
        inject_frame(ACK, {}, 0U);
        Frame::Frame frame;
        Receive_result result = slave.receive_request(&frame, 0U);
        CHECK("Handle handshake", result == Smart_serial::HANDLED, &pass, &fail);
    }

    {
        // TEST 2: WRONG ADDRESS
        inject_frame(ACK, {}, 0U, 0xAA); // Wrong address
        Frame::Frame frame;
        Receive_result result = slave.receive_request(&frame, 0U);
        CHECK("Receive wrong adress", result == ERR_WRONG_ADDRESS, &pass, &fail);
    }

    {
        // TEST 3: Nullptr passed
        inject_frame(ACK, {}, 0U);
        Frame::Frame frame;
        Receive_result result = slave.receive_request(nullptr, 0U);
        CHECK("Process error (nullptr passed)", result == Smart_serial::ERR_PROCESS, &pass, &fail);
    }

    {
        // TEST 4: NACK
        inject_frame(NACK, {}, 0U);
        Frame::Frame frame;
        Receive_result result = slave.receive_request(&frame, 0U);
        CHECK("Receive NACK", result == ERR_NACK, &pass, &fail);
    }

    {
        // TEST 5: CRC ERR
        inject_frame(ACK, {}, 0U, 0x02U, 21U);
        Frame::Frame frame;
        Receive_result result = slave.receive_request(&frame, 0U);
        CHECK("Receive NACK", result == Smart_serial::ERR_CRC, &pass, &fail);
    }

    {
        // TEST 6: Receive successfully
        slave.set_auto_handshake(false);
        inject_frame(NACK, {}, 0U);
        Frame::Frame frame;
        Receive_result result = slave.receive_request(&frame, 0U);
        CHECK("Receive NACK", result == ERR_NACK, &pass, &fail);
    }
    

    
    

    printf("%i/%i passed", pass, pass+fail);

    return (fail == 0);
}