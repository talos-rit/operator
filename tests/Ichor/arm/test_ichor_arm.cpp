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
        ichorArm = new Ichor("/dev/i2c0", 0x10, 0x11);
    }
    
    void teardown()
    {
        delete ichorArm;
        mock().clear();
    }
};

// TODO: Add more tests for Ichor arm functionality
IGNORE_TEST(IchorArmTest, Initialization)
{
    // TODO: Mock the open system call to simulate device opening
    CHECK_TRUE(ichorArm != NULL);
}
