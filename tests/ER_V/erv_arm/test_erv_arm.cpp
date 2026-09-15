#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include "erv_arm/erv.hpp"
#include "acl/acl.hpp"

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

TEST_GROUP(ACLTest) {};

TEST(ACLTest, JointJogMapsShoulderAndElbowToManualCharacters)
{
    CHECK_EQUAL('w', ACL_get_joint_jog_vector(2, -1));
    CHECK_EQUAL('2', ACL_get_joint_jog_vector(2, 1));
    CHECK_EQUAL('e', ACL_get_joint_jog_vector(3, -1));
    CHECK_EQUAL('3', ACL_get_joint_jog_vector(3, 1));
    CHECK_EQUAL('\0', ACL_get_joint_jog_vector(1, 1));
    CHECK_EQUAL('\0', ACL_get_joint_jog_vector(2, 0));
}
