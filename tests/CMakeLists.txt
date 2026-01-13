# tests/CMakeLists.txt

# CppUTest for testing
find_package(CppUTest QUIET)
if(NOT CppUTest_FOUND)
    message(STATUS "CppUTest not found, tests will be disabled")
    set(BUILD_TESTS OFF)
else()
    set(BUILD_TESTS ON)
endif()

option(ENABLE_COVERAGE "Enable code coverage for tests. Requires Debug build type. Automatically applied for unit testing" OFF)
option(ENABLE_REPORT "Enable generation of coverage report after tests. Requires ENABLE_COVERAGE" OFF)

# Coverage flags (only for Debug builds)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage -lgcov")
endif()

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
        common_test_exe
        erv_test_exe
        ichor_test_exe
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
    DEPENDS common_test_exe erv_test_exe ichor_test_exe
)

# Coverage report target
if(ENABLE_COVERAGE AND ENABLE_REPORT AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    find_program(LCOV_PATH lcov)
    find_program(GENHTML_PATH genhtml)
    
    if(LCOV_PATH AND GENHTML_PATH)
        add_custom_target(coverage_report
            COMMAND ${LCOV_PATH} --capture 
                    --directory ${CMAKE_BINARY_DIR}/obj 
                    --output-file ${CMAKE_BINARY_DIR}/coverage.info 
                    --exclude '*/12/*' 
                    --exclude '/usr/include/*' 
                    --exclude '*/tests/*'
            COMMAND ${GENHTML_PATH} ${CMAKE_BINARY_DIR}/coverage.info 
                    --output-directory ${CMAKE_BINARY_DIR}/coverage
            COMMAND echo "Coverage report generated at ${CMAKE_BINARY_DIR}/coverage/index.html"
            COMMENT "Generating coverage report"
            DEPENDS test_all
        )
    endif()
endif()