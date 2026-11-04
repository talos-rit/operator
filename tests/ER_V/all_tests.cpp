#include <CppUTest/CommandLineTestRunner.h>
#include <CppUTest/TestHarness.h>

TEST_GROUP(Group){};

IGNORE_TEST(Group, Test){};

int main(int argc, char **argv) { return RUN_ALL_TESTS(argc, argv); }
