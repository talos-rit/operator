cmake_minimum_required(VERSION 3.20)
project(operator VERSION 1.0.0 LANGUAGES C CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Generate compile_commands.json for IDEs (VS Code IntelliSense)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compiler options matching original Makefile
add_compile_options(
    -Wall
    -Wextra
    -Wno-deprecated-declarations
)

add_compile_definitions(_DEFAULT_SOURCE)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Output directories (matching original build/bin and build/obj structure)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Find pthread
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Option to build tests
option(BUILD_TESTING "Build the testing tree" ON)

# # Dependencies for proto/gRPC code (provided by Conan when installed)
# find_package(protobuf CONFIG QUIET)
# find_package(gRPC CONFIG QUIET)

# Coverage flags (only for Debug builds)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage -lgcov")
endif()

# Add subdirectories
# add_subdirectory(src/proto)
add_subdirectory(src/common)
add_subdirectory(src/ER_V)
add_subdirectory(src/Ichor)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()