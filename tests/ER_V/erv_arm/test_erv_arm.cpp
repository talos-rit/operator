#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include "erv_arm/erv.hpp"

TEST_GROUP(ScorbotTest)
{
    Scorbot* scorbot;
    
    void setup()
    {
        scorbot = new Scorbot("/dev/ttyS0");
    }
    
    void teardown()
    {
        delete scorbot;
        mock().clear();
    }
};

// TODO: Add more tests for Scorbot class methods
IGNORE_TEST(ScorbotTest, Initialization)
{
    // TODO: Mock the open system call to simulate device opening
    CHECK_TRUE(scorbot != NULL);
}