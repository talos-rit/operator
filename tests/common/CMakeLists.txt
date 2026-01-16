# tests/common/CMakeLists.txt
# Common test sources
set(COMMON_TEST_SRCS
    conf/test_config_sys.cpp
    conf/test_config_parsing.cpp
    data/s_list_test.cpp
    log/test_log_config.cpp
    socket/test_socket_config.cpp
    all_tests.cpp 
)

add_executable(common_test_exe
    ${COMMON_TEST_SRCS}
)

target_link_libraries(common_test_exe
    common_objs
    CppUTest
    CppUTestExt
    Threads::Threads
)

add_test(NAME common_tests COMMAND common_test_exe)

add_custom_target(test_common
    DEPENDS common_test_exe
    COMMAND ./common_test_exe
    COMMENT "Running common tests"
)