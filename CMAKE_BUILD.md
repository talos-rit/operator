# CMake Build Instructions

This project has been migrated from Make to CMake with Conan for dependency management.

## Prerequisites

- CMake 3.20 or higher
- Conan 2.x
- C++ compiler with C++20 support (g++ recommended)
- Git

## Install Conan (if not already installed)

```bash
pip install conan
```

## Build Steps

### 1. Install Dependencies with Conan

```bash
# Install dependencies and generate CMake toolchain
conan install . --output-folder=build --build=missing
```

### 2. Configure CMake

```bash
# Using CMake presets (recommended)
cmake --preset default

# OR manually without presets
cmake -B build -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug
```

### 3. Build

```bash
# Build all targets
cmake --build build

# OR build specific target
cmake --build build --target erv
cmake --build build --target ichor
cmake --build build --target dummy
```

### 4. Run Executables

The executables will be in `build/bin/`:

```bash
./build/bin/erv
./build/bin/ichor
./build/bin/dummy
```

### 5. Run Tests (optional)

```bash
# Run all tests
ctest --test-dir build

# OR run specific test executable
./build/bin/erv_test
./build/bin/ichor_test
```

## Build Configurations

### Debug Build (default)
```bash
cmake --preset default
cmake --build build
```

### Release Build
```bash
cmake --preset release
cmake --build build
```

## Clean Build

```bash
rm -rf build
```

## Targets

- `erv` - ER_V executable
- `ichor` - Ichor executable
- `dummy` - Dummy test executable
- `erv_test` - ER_V unit tests (requires BUILD_TESTING=ON)
- `ichor_test` - Ichor unit tests (requires BUILD_TESTING=ON)
- `common` - Common static library (built automatically)
- `driver` - Driver static library (built automatically)

## Disable Tests

To build without tests:

```bash
conan install . --output-folder=build --build=missing
cmake -B build -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake -DBUILD_TESTING=OFF
cmake --build build
```

## Cross-compilation for Raspberry Pi (Advanced)

For cross-compiling to Raspberry Pi (e.g., for the Ichor version with PREEMPT_RT):

1. Create a Conan profile for your target (e.g., `rpi_profile`)
2. Use the profile during installation:
   ```bash
   conan install . --output-folder=build --build=missing --profile:host=rpi_profile
   ```
3. Configure CMake with the appropriate toolchain file

## Migration Notes

- The original Makefile and .mk files are preserved for reference
- Build output structure maintained: `build/bin/` for executables
- All compiler flags from the original Makefile are preserved
- CppUTest dependency now managed by Conan
