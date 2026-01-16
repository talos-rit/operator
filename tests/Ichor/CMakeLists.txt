# tests/Ichor/CMakeLists.txt
add_executable(ichor_test_exe
    all_tests.cpp
    arm/test_ichor_arm.cpp
    conf/test_ichor_conf.cpp
)

target_link_libraries(ichor_test_exe
    common_objs
    driver_objs
    $<TARGET_OBJECTS:ichor>
    CppUTest::CppUTest
    CppUTest::CppUTestExt
    Threads::Threads
)

add_test(NAME ichor_test COMMAND ichor_test_exe)

add_custom_target(test_ichor
    DEPENDS ichor_test_exe
    COMMAND ./ichor_test_exe
    COMMENT "Running Ichor tests"
)