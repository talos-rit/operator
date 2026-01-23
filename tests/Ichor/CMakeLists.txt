# tests/Ichor/CMakeLists.txt
add_executable(test_ichor_exe
    all_tests.cpp
    arm/test_ichor_arm.cpp
    conf/test_ichor_conf.cpp
)

target_link_libraries(test_ichor_exe
    common_objs
    driver
    ichor_objs
    CppUTest
    CppUTestExt
    Threads::Threads
)

add_test(NAME test_ichor
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_ichor_exe)

add_custom_target(test_ichor
    DEPENDS test_ichor_exe
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_ichor_exe
    COMMENT "Running Ichor tests"
)