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

TEST(ScorbotTest, Initialization)
{
    CHECK_TRUE(scorbot != NULL);
}