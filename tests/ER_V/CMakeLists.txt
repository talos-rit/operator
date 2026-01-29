# tests/ER_V/CMakeLists.txt
add_executable(test_erv_exe
    all_tests.cpp
    erv_arm/test_erv_arm.cpp
    erv_conf/test_erv_config.cpp
)

target_link_libraries(test_erv_exe
    common_objs
    erv_objs
    CppUTest
    CppUTestExt
    Threads::Threads
)

add_test(NAME test_erv
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_erv_exe)

add_custom_target(test_erv
    DEPENDS test_erv_exe
    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_erv_exe
    COMMENT "Running ER_V tests"
)

set(ERV_REPORT_DIR ${CMAKE_BINARY_DIR}/test_reports/erv)

add_custom_target(test_erv_report
    DEPENDS test_erv_exe
    COMMAND mkdir -p ${ERV_REPORT_DIR}
    COMMAND echo "${CMAKE_BINARY_DIR}"
	COMMAND cd ${ERV_REPORT_DIR} && ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/test_erv_exe -ojunit
    COMMENT "Running ER_V tests with JUnit report"
)