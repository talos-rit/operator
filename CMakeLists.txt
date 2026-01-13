# ./CMakeLists.txt
cmake_minimum_required(VERSION 3.25)
project(TalosOperator
    LANGUAGES CXX
    DESCRIPTION "Operator for ER V and Ichor (ER 4PC) devices"
)

# Options to build components
option(ENABLE_CPPCHECK "Enable Cppcheck static analysis" OFF)
option(ENABLE_CLANG_TIDY "Enable Clang-Tidy static analysis" OFF)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Versioning File Generation
# configure_file(${CMAKE_SOURCE_DIR}/version.h.in ${CMAKE_SOURCE_DIR}/src/common/version.h)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-deprecated-declarations -D _DEFAULT_SOURCE")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-deprecated-declarations -D _DEFAULT_SOURCE")

# Static analysis
if(ENABLE_CPPCHECK)
    find_program(CPPCHECK_EXE cppcheck)
    if(CPPCHECK_EXE)
        message(STATUS "Cppcheck found: ${CPPCHECK_EXE}")
        # Flags for erv and ichor will need to be added later
        set(CMAKE_CXX_CPPCHECK "${CPPCHECK_EXE} --enable=all --inconclusive --std=c++20 --language=c++ --platform=unix64")
    else()
        message(WARNING "Cppcheck not found, static analysis using cppcheck will be skipped")
    endif()
endif()

if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE run-clang-tidy)
    if(CLANG_TIDY_EXE)
        message(STATUS "Clang-Tidy found: ${CLANG_TIDY_EXE}")
        # Flags for erv and ichor will need to be added later
        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE} -format")
    else()
        message(WARNING "Clang-Tidy not found, static analysis using clang-tidy will be skipped")
    endif()
endif()

# Include find pthreads library so it can be included
find_package(Threads REQUIRED)

# Common include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src/common
    # ${CMAKE_SOURCE_DIR}/src/ER_V
    # ${CMAKE_SOURCE_DIR}/src/Ichor
)

# Add subdirectories
add_subdirectory(src/common)
add_subdirectory(src/ER_V)
add_subdirectory(src/Ichor)
# add_subdirectory(src/)

# Add tests cmake file if tests are enabled or targeted
if (BUILD_TESTS)
    add_subdirectory(tests)
endif()

# Custom targets for convenience
add_custom_target(all_targets
    DEPENDS erv ichor
    COMMENT "Building all main targets"
)

add_custom_target(clean_all
    COMMAND ${CMAKE_COMMAND} -E rm -r -- ${CMAKE_BINARY_DIR}
    COMMENT "Cleaning all build files"
)
