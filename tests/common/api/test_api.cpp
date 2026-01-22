#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include "api/api.hpp"
#include <endian.h>
#include <iostream>

// Test group for API functions
TEST_GROUP(APITest)
{
    void setup()
    {
        // Setup code
    }
    void teardown()
    {
        // Teardown code
        mock().clear();
    }
};

// prep_polar_pan tests
IGNORE_TEST(APITest, PrepPolarPan_ValidInput)
{
    API::PolarPan pan;
    pan.delta_azimuth = htobe32(1000);
    pan.delta_altitude = htobe32(2000);
    pan.delay_ms = htobe32(500);
    pan.time_ms = htobe32(1500);

    // int result = API::prep_polar_pan(&pan);
    int result = -1; // Placeholder for actual function call
    CHECK_EQUAL(0, result);
    CHECK_EQUAL(1000, pan.delta_azimuth);
    CHECK_EQUAL(2000, pan.delta_altitude);
    CHECK_EQUAL(500, pan.delay_ms);
    CHECK_EQUAL(1500, pan.time_ms);
}

IGNORE_TEST(APITest, PrepPolarPan_NullInput)
{
    // int result = API::prep_polar_pan(nullptr);
    int result = 0; // Placeholder for actual function call
    CHECK_EQUAL(-1, result);
}

// prep_home tests
IGNORE_TEST(APITest, PrepHome_ValidInput)
{
    API::Home home;
    home.delay_ms = htobe32(300);
    // int result = API::prep_home(&home);
    int result = -1; // Placeholder for actual function call
    CHECK_EQUAL(0, result);
    CHECK_EQUAL(300, home.delay_ms);
}

IGNORE_TEST(APITest, PrepHome_NullInput)
{
    // int result = API::prep_home(nullptr);
    int result = 0; // Placeholder for actual function call
    CHECK_EQUAL(-1, result);
}

// validate_command tests
TEST(APITest, ValidateCommand_NullBuffer)
{
    int result = API::validate_command(nullptr, 10);
    CHECK_EQUAL(-1, result);
}

TEST(APITest, ValidateCommand_InsufficientLength)
{
    uint8_t buffer[sizeof(API::DataHeader) - 1] = {0};
    int result = API::validate_command(buffer, 1);
    CHECK_EQUAL(-1, result);
}

TEST(APITest, ValidateCommand_ValidHandshakeCommand)
{
    // Prepare a valid Handshake command buffer
    API::DataHeader header;
    header.msg_id = htobe32(1);
    header.reserved_1 = 0;
    header.cmd_id = htobe16(static_cast<uint16_t>(API::CommandID::Handshake));
    header.len = htobe16(0);
    uint8_t buffer[sizeof(API::DataHeader)];
    memcpy(buffer, &header, sizeof(API::DataHeader));

    int result = API::validate_command(buffer, sizeof(buffer));
    CHECK_EQUAL(0, result);
}

TEST(APITest, ValidateCommand_ValidPolarPanStartCommand)
{
    // Prepare a valid PolarPanStart command buffer
    API::DataHeader header;
    header.msg_id = htobe32(1);
    header.reserved_1 = 0;
    header.cmd_id = htobe16(static_cast<uint16_t>(API::CommandID::PolarPanStart));
    header.len = htobe16(0);
    uint8_t *buffer = reinterpret_cast<uint8_t *>(&header);

    int result = API::validate_command(buffer, sizeof(API::DataHeader));
    CHECK_EQUAL(0, result);
}

TEST(APITest, ValidateCommand_ValidPolarPanStopCommand)
{
    // Prepare a valid PolarPanStop command buffer
    API::DataHeader header;
    header.msg_id = htobe32(1);
    header.reserved_1 = 0;
    header.cmd_id = htobe16(static_cast<uint16_t>(API::CommandID::PolarPanStop));
    header.len = htobe16(0);
    uint8_t *buffer = reinterpret_cast<uint8_t *>(&header);

    int result = API::validate_command(buffer, sizeof(API::DataHeader));
    CHECK_EQUAL(0, result);
}

TEST(APITest, ValidateCommand_ValidPolarPanCommand)
{
    // Prepare a valid PolarPan command buffer
    API::DataHeader header;
    header.msg_id = htobe32(1);
    header.reserved_1 = 0;
    header.cmd_id = htobe16(static_cast<uint16_t>(API::CommandID::PolarPan));
    header.len = htobe16(sizeof(API::PolarPan));

    API::DataWrapper wrapper;
    memcpy(&wrapper.header, &header, sizeof(API::DataHeader));
    
    auto *payload = reinterpret_cast<API::PolarPan *>(&wrapper.payload_head);
    payload->delta_azimuth = htobe32(1000);
    payload->delta_altitude = htobe32(2000);
    payload->delay_ms = htobe32(500);
    payload->time_ms = htobe32(1500);

    uint8_t *buffer = reinterpret_cast<uint8_t *>(&wrapper);

    int result = API::validate_command(buffer, sizeof(API::DataHeader) + sizeof(API::PolarPan));
    CHECK_EQUAL(0, result);

    // Verify that the payload has been prepared correctly
    CHECK_EQUAL(1000, payload->delta_azimuth);
    CHECK_EQUAL(2000, payload->delta_altitude);
    CHECK_EQUAL(500, payload->delay_ms);
    CHECK_EQUAL(1500, payload->time_ms);
}

TEST(APITest, ValidateCommand_ValidHomeCommand)
{
    // Prepare a valid Home command buffer
    API::DataHeader header;
    header.msg_id = htobe32(1);
    header.reserved_1 = 0;
    header.cmd_id = htobe16(static_cast<uint16_t>(API::CommandID::Home));
    header.len = htobe16(sizeof(API::Home));

    API::DataWrapper wrapper;
    memcpy(&wrapper.header, &header, sizeof(API::DataHeader));
    
    auto *payload = reinterpret_cast<API::Home *>(&wrapper.payload_head);
    payload->delay_ms = htobe32(300);
    uint8_t *buffer = reinterpret_cast<uint8_t *>(&wrapper);

    int result = API::validate_command(buffer, sizeof(API::DataHeader) + sizeof(API::Home));
    CHECK_EQUAL(0, result);

    // Verify that the payload has been prepared correctly
    CHECK_EQUAL(300, payload->delay_ms);
}

TEST(APITest, ValidateCommand_InvalidCommandID)
{
    // Prepare a command buffer with an invalid command ID
    API::DataHeader header;
    header.msg_id = htobe32(1);
    header.reserved_1 = 0;
    header.cmd_id = htobe16(0xFFFF); // Invalid command ID
    header.len = htobe16(0);
    uint8_t *buffer = reinterpret_cast<uint8_t *>(&header);

    int result = API::validate_command(buffer, sizeof(API::DataHeader));
    CHECK_EQUAL(-1, result);
}