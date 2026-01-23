# tests/Ichor/CMakeLists.txt
add_executable(ichor_test_exe
    all_tests.cpp
    arm/test_ichor_arm.cpp
    conf/test_ichor_conf.cpp
)

target_link_libraries(ichor_test_exe
    common_objs
    driver
    ichor_objs
    CppUTest
    CppUTestExt
    Threads::Threads
)

add_test(NAME ichor_test
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ichor_test_exe)

add_custom_target(test_ichor
    DEPENDS ichor_test_exe
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/ichor_test_exe
    COMMENT "Running Ichor tests"
)