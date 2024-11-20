// CPPUTEST testing module

#include "CppUTest/TestHarness.h"
#include "tmp/tmp.h"

TEST_GROUP(FirstTestGroup)
{
};

TEST(FirstTestGroup, FirstTest)
{
    //noop
}

TEST(FirstTestGroup, SecondTest)
{
    int i = -16;
    for (; i < 0; i++)
    {
        CHECK_EQUAL(-1, test(i));
    }

    for (; i < 16; i++)
    {
        CHECK_EQUAL(i, test(i));
    }
}