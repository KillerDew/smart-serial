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
#include <cstring>

using namespace Smart_serial;
using namespace Smart_serial::CRC;
using namespace Smart_serial::Clock;

int Tests::test_slave() {

    printf("-- Testing slave device implementations --\n");
    Mock_port<256, 256> port;
    Std_clock clock;
    Slave slave(port, clock, 0x02U, 0xFE, 0xAAU, 1000U);
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
        CHECK("Receive CRC error", result == Smart_serial::ERR_CRC, &pass, &fail);
    }

    {
        // TEST 6: Receive successfully
        slave.set_auto_handshake(false);
        inject_frame(NACK, {}, 0U);
        Frame::Frame frame;
        Receive_result result = slave.receive_request(&frame, 0U);
        CHECK("Receive success", result == ERR_NACK, &pass, &fail);
    }

    {
        // Test 7: send buffer
        port.reset();
        const uint8_t buf[] = {0x00U, 0x01U, 0x02U};

        slave.send_response(buf, 3U, ACK);

        const uint8_t*  tx_buf = port.get_tx_data();
        const uint16_t  tx_len = port.get_tx_data_len();

        uint8_t tx_buf_copy[Frame::S_SERIAL_MAX_FRAME_BYTES] = {};
        static_cast<void>(memcpy(tx_buf_copy, tx_buf, static_cast<size_t>(tx_len)));

        Frame::Frame expected_frame;
        Frame::build_frame(&expected_frame,
                        0xAAU,
                        0x02U,
                        0xFEU,
                        ACK,
                        buf,
                        3U);

        Frame::Raw_frame expected_raw;
        Frame::dump_frame(&expected_raw, &expected_frame);

        // Compute and append CRC over the frame data excluding CRC bytes
        const uint16_t crc = CRC::compute_crc16(expected_raw.data,
                                                static_cast<uint16_t>(expected_raw.length));

        static_cast<void>(CRC::append_crc16(&expected_raw,
                                            Frame::S_SERIAL_MAX_FRAME_BYTES,
                                            static_cast<uint16_t>(expected_raw.length),
                                            crc));

        uint8_t expected_buf[Frame::S_SERIAL_MAX_FRAME_BYTES] = {};
        static_cast<void>(memcpy(expected_buf,
                                expected_raw.data,
                                expected_raw.length));

        CHECK("Send byte buffer",
            (memcmp(tx_buf_copy,
                    expected_buf,
                    static_cast<size_t>(tx_len)) == 0),
            &pass, &fail);
    }
    {
        // Test 8: send string
        port.reset();
        const char* str = "hello";

        slave.send_response(str, ACK);

        const uint8_t*  tx_buf = port.get_tx_data();
        const uint16_t  tx_len = port.get_tx_data_len();

        uint8_t tx_buf_copy[Frame::S_SERIAL_MAX_FRAME_BYTES] = {};
        static_cast<void>(memcpy(tx_buf_copy, tx_buf, static_cast<size_t>(tx_len)));

        Frame::Frame expected_frame;
        Frame::build_frame(&expected_frame,
                        0xAAU,
                        0x02U,
                        0xFEU,
                        ACK,
                        reinterpret_cast<const uint8_t*>(str),
                        strlen(str));

        Frame::Raw_frame expected_raw;
        Frame::dump_frame(&expected_raw, &expected_frame);

        const uint16_t crc = CRC::compute_crc16(expected_raw.data,
                                                static_cast<uint16_t>(expected_raw.length));

        static_cast<void>(CRC::append_crc16(&expected_raw,
                                            Frame::S_SERIAL_MAX_FRAME_BYTES,
                                            static_cast<uint16_t>(expected_raw.length),
                                            crc));

        uint8_t expected_buf[Frame::S_SERIAL_MAX_FRAME_BYTES] = {};
        static_cast<void>(memcpy(expected_buf,
                                expected_raw.data,
                                expected_raw.length));

        CHECK("Send string buffer",
            (memcmp(tx_buf_copy,
                    expected_buf,
                    static_cast<size_t>(tx_len)) == 0),
            &pass, &fail);
    }

    printf("%i/%i passed\n--Done Testing slave implementations--\n", pass, pass+fail);

    return (fail == 0);
}