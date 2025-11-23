#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include "arm/ichor_arm.h"

TEST_GROUP(IchorArmTest)
{
    Ichor* ichorArm;
    
    void setup()
    {
        // TODO: Provide appropriate constructor arguments
        // const char *isr_dev, const char *i2c_dev, uint8_t dac0_addr, uint8_t dac1_addr, uint8_t adc_addr
        ichorArm = new Ichor("/dev/ichor0", "/dev/i2c0", 0x10, 0x11, 0x12);
    }
    
    void teardown()
    {
        delete ichorArm;
        mock().clear();
    }
};

TEST(IchorArmTest, Initialization)
{
    CHECK_TRUE(ichorArm != NULL);
}

// TODO: Add more tests for Ichor arm functionality

