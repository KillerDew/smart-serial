#include "smart-serial/clock/Std_clock.hpp"
#include "smart-serial/port/mock_port.hpp"
#include "smart-serial/master.hpp"
#include "smart-serial/crc.hpp"
#include "smart-serial/frame.hpp"
#include "tests/tests.hpp"
#include <cstdint>
#include <cstdio>
using namespace std;
using namespace Smart_serial;
using namespace Smart_serial::Clock;
using namespace Smart_serial::CRC;
int Tests::test_master() {
    printf("-- Testing master device implementations --\n");
    Mock_port<256, 256> port;
    Std_clock clock;
    Master master(port, clock, 0x02U, 1000U);

    int pass = 0, fail = 0;
    #define CHECK(label, cond) \
        if (cond) { printf("PASS - %s\n", label); ++pass; } \
        else      { printf("FAIL - %s\n", label); ++fail; }

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

    // Test 1: receive a valid ACK frame
    {
        inject_frame(ACK, nullptr, 0U);
        Frame::Frame frame{};
        Receive_result res = master.receive_packet(&frame, 1000U);
        CHECK("receive ACK", res == SUCCESS);
        port.reset();
    }

    // Test 2: receive a NACK frame
    {
        inject_frame(NACK, nullptr, 0U);
        Frame::Frame frame{};
        Receive_result res = master.receive_packet(&frame, 1000U);
        CHECK("receive NACK", res == ERR_NACK);
        port.reset();
    }

    // Test 3: null frame_out guard
    {
        Receive_result res = master.receive_packet(nullptr, 1000U);
        CHECK("null frame_out", res == ERR_PROCESS);
    }

    // Test 4: handshake succeeds
    {
        inject_frame(ACK, nullptr, 0U);
        Receive_result res = master.handshake(3U, 1000U);
        CHECK("handshake", res == SUCCESS);
        port.reset();
    }

    // Test 5: receive frame with payload
    {
        const uint8_t payload[] = {0x11, 0x22, 0x33};
        inject_frame(0x42U, payload, sizeof(payload));
        Frame::Frame frame{};
        Receive_result res = master.receive_packet(&frame, 1000U);
        CHECK("receive with payload", res == SUCCESS && frame.header.payload_len == 3U);
        port.reset();
    }
    // Test 6: no response timeout (very short timeout)
    Frame::Frame frame{};
    Receive_result res = master.receive_packet(&frame, 1U); // 1ms timeout
    CHECK("timeout", res == ERR_TIMEOUT);
    port.reset();

    printf("\n%d/%d passed\n -- Done testing master implementations --\n", pass, pass + fail);
    return (fail == 0) ? 0 : 1;
}