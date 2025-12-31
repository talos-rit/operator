# Operator

## Prerequisites

### Linux
- CMake 3.20 or higher
- C/C++ compiler (GCC 9+ or Clang 10+)
- Python 3.8+ (for Conan)
- Git

Install on Debian/Ubuntu:
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake git python3 python3-pip
pip3 install conan
```

### macOS/Windows
Use Dev Containers (see [Development Setup](#development-setup) section).

## Initial Setup

1. Clone the repository with `--recursive` to get the driver submodule:
```bash
git clone --recursive <repo-url>
cd operator
```

2. Install Conan dependencies:
```bash
conan install . --output-folder=build --build=missing -s build_type=Debug
conan install . --output-folder=build --build=missing -s build_type=Release
```

This downloads and builds all required dependencies (gRPC, protobuf, CppUTest, etc.) and generates CMake configuration files.

## Building

### Configure (one-time setup)
```bash
cmake --preset conan-debug
```

Or for Release:
```bash
cmake --preset conan-release
```

### Compile
```bash
cmake --build --preset conan-debug -j
```

For Release:
```bash
cmake --build --preset conan-release -j
```

The compiled binaries are in `build/bin/`:
- `erv` - Main ER_V executable
- `dummy` - Dummy test runner
- `erv_test` - ER_V unit tests
- `ichor_test` - Ichor unit tests

## Running

### Run executables
```bash
./build/bin/erv
./build/bin/dummy
```

### Run tests

Run the CppUTest test executables directly:

```bash
./build/bin/erv_test
./build/bin/ichor_test
```

## Development Setup

### Linux/Dev Container
A Linux machine is required to compile the operator. If you are on macOS or Windows, use Dev Containers as described below.

### macOS/Windows with Dev Containers

1. Install [Docker](https://www.docker.com/get-started/) and [VS Code](https://code.visualstudio.com/).

2. Install the [Dev Containers extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) in VS Code. 
   *Note: The installed Docker version must be compatible with the Dev Containers extension. Check the [documentation](https://code.visualstudio.com/docs/devcontainers/containers#_prerequisites) for compatibility.*

3. Clone the repository:
```bash
git clone --recursive <repo-url>
```

4. Open the cloned repository folder in VS Code.

5. Open the Command Palette (`Ctrl+Shift+P` or `Cmd+Shift+P` on macOS) and select **Dev Containers: Reopen in Container**.

6. VS Code will build and start the dev container (first time takes a few minutes). Once ready, you can run the build commands above.

## Project Structure

```
operator/
├── src/
│   ├── common/          # Common utilities and data structures
│   ├── ER_V/            # ER_V subsystem (main executable)
│   ├── Ichor/           # Ichor subsystem (driver integration)
│   │   └── driver/      # Driver library
│   └── proto/           # gRPC service definitions (pre-generated)
├── CMakeLists.txt       # Main build configuration
├── conanfile.py         # Conan dependency management
├── CMakeUserPresets.json # Conan-generated presets (auto-generated)
└── build/               # Build output (ignored in git)
```

## Troubleshooting

### CMake can't find protobuf/gRPC
Ensure Conan installed successfully:
```bash
conan install . --output-folder=build --build=missing -s build_type=Debug
```

### Compilation errors about missing headers
Run CMake configure again:
```bash
cmake --preset conan-debug --fresh
```

### Tests fail
Verify dependencies compiled correctly:
```bash
ls -la build/build/Debug/generators/
```
This directory should contain CMake config files from Conan.
