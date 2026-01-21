#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include "conf/ichor_conf.h"

TEST_GROUP(IchorConfigTest)
{
    IchorConfig* ichorConfig;
    
    void setup()
    {
        ichorConfig = new IchorConfig();
    }
    
    void teardown()
    {
        delete ichorConfig;
        mock().clear();
    }
};

TEST(IchorConfigTest, Initialization)
{
    CHECK_TRUE(ichorConfig != NULL);
    
    // Test default I2C device path
    const char* i2c_dev = ichorConfig->GetI2CDev();
    STRCMP_EQUAL("/dev/i2c-1", i2c_dev);
}

// TODO: Add more tests for IchorConfig functionality