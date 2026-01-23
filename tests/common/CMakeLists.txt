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

add_executable(test_common_exe
    ${COMMON_TEST_SRCS}
)

target_link_libraries(test_common_exe
    common_objs
    CppUTest
    CppUTestExt
    Threads::Threads
)

add_test(NAME test_common
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_common_exe)

add_custom_target(test_common
    DEPENDS test_common_exe
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_common_exe
    COMMENT "Running common tests"
)

set(COMMON_REPORT_DIR ${CMAKE_BINARY_DIR}/test_reports/common)

add_custom_target(test_common_report
    DEPENDS test_common_exe
    COMMAND mkdir -p ${COMMON_REPORT_DIR}
    COMMAND echo "${CMAKE_BINARY_DIR}"
	COMMAND cd ${COMMON_REPORT_DIR} && ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_common_exe -ojunit
    COMMENT "Running common tests with JUnit report"
)