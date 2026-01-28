# CppUTest for testing
find_package(PkgConfig REQUIRED)
pkg_search_module(CPPUTEST REQUIRED cpputest>=3.8)
message(STATUS "Found CppUTest version ${CPPUTEST_VERSION}")

include_directories(${CPPUTEST_INCLUDE_DIRS} ../src/)
link_directories(${CPPUTEST_LIBRARIES})

# Test output directories
set(TEST_BIN_DIR ${CMAKE_BINARY_DIR}/bin/tests)
set(TEST_REPORT_DIR ${CMAKE_BINARY_DIR}/reports)

enable_testing()

# Test targets
add_subdirectory(common)
add_subdirectory(ER_V)
add_subdirectory(Ichor)

# ============================================================================
# CTest Configuration
# ============================================================================

# Set CTest options
set(CTEST_OUTPUT_ON_FAILURE ON)  # Show output when tests fail
set(CTEST_USE_LAUNCHERS ON)      # Better IDE integration

# ============================================================================
# Aggregate Targets (for convenience)
# ============================================================================
add_custom_target(build_tests
    DEPENDS
        test_common_exe
        test_erv_exe
        test_ichor_exe
    COMMENT "Building all test executables"
)

# Test discovery and reporting
add_custom_target(test_all
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS build_tests
    COMMENT "Running all tests"
)

# Test discovery and reporting
add_custom_target(test_verbose
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS build_tests
    COMMENT "Running tests with verbose output"
)

# ============================================================================
# Test Reports
# ============================================================================
# Test with Junit output
add_custom_target(test_all_report
    COMMAND ${CMAKE_CTEST_COMMAND} --output-junit ${CMAKE_BINARY_DIR}/reports/test-results.xml
    COMMENT "Running tests with JUnit output"
    DEPENDS test_common_exe test_erv_exe test_ichor_exe
)

# Coverage report target
find_program(LCOV_PATH lcov)
find_program(GENHTML_PATH genhtml)

add_custom_target(coverage_report
    COMMAND ${LCOV_PATH} --capture 
            --directory ${CMAKE_BINARY_DIR}
            --output-file ${CMAKE_BINARY_DIR}/coverage.info 
            --exclude '*/12/*' 
            --exclude '/usr/include/*' 
            --exclude '*/tests/*'
    COMMAND ${GENHTML_PATH} ${CMAKE_BINARY_DIR}/coverage.info 
            --output-directory ${CMAKE_BINARY_DIR}/coverage_report
    COMMAND echo "Coverage report generated at ${CMAKE_BINARY_DIR}/coverage_report/index.html"
    COMMENT "Generating coverage report"
    DEPENDS test_all
)