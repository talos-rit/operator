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

TEST(ACLTest, EnableControlAndSpeedFormatDocumentedAclCommands)
{
    CHECK_EQUAL(0, ACL_init());
    S_List commands;
    DATA_S_List_init(&commands);

    CHECK_EQUAL(0, ACL_enqueue_enable_control_cmd(&commands));
    CHECK_EQUAL(0, ACL_enqueue_speed_percent_cmd(&commands, 20));
    CHECK_EQUAL(-1, ACL_enqueue_speed_percent_cmd(&commands, 0));

    S_List_Node* node = DATA_S_List_pop(&commands);
    ACL_Command* command = DATA_LIST_GET_OBJ(node, ACL_Command, node);
    STRCMP_EQUAL("CON\r", command->payload);
    ACL_Command_init(command);

    node = DATA_S_List_pop(&commands);
    command = DATA_LIST_GET_OBJ(node, ACL_Command, node);
    STRCMP_EQUAL("SPEED 20\r", command->payload);
    ACL_Command_init(command);
    CHECK_EQUAL(0, ACL_destroy());
}
