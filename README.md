# Operator


## Initial Setup
1. Clone the repo with `--recursive`to get the driver submodule:
```bash
git clone --recursive <repo-url>
```
*Note: Replace `<repo-url>` with the actual URL of the repository.*

## Compilation and Development
A Linux machine is required to develop and compile the operator. If you are on `macOS` or `Windows`, skip to [Additional Steps for macOS/Windows](#additional-steps-for-macoswindows).

### Compilation
The command to compile the operator is:
```bash
make erv
```
To compile the operator with the driver for the ER 4pc, also known as `ichor`, use:
```bash
make ichor
```

### Additional steps for macOS/Windows
If you are on `macOS` or `Windows`, it is impossible to compile the operator/driver directly. This makes development challenging since you are unable to use all the features of your IDE during development. You can use a Linux virtual machine (VM) or Docker to compile the operator. Below are the steps to set up a Dev Container for development. It is recommended to use the Dev Containers extension for VS Code for a smoother experience.

1. Install [Docker](https://www.docker.com/get-started/) and [VS Code](https://code.visualstudio.com/).
2. Install the [Dev Containers extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode-remote.remote-containers) in VS Code. *Note: The installed Docker version needs to be compatible with the Dev Containers extension. Check the [documentation](https://code.visualstudio.com/docs/devcontainers/containers#_prerequisites) for more information.*
3. Open the cloned repository in VS Code.
4. Open the Command Palette (`Ctrl+Shift+P` or `Cmd+Shift+P` on macOS) and select `Dev Containers: Open Folder in Container`.

### Running Tests
The tests are written using the `Cpputest` unit test framework. To learn more about `Cpputest` see the [Cpputest documentation](https://cpputest.github.io/manual.html). The test suite is split up into three different parts that are executed separately for each of the major components of operator and Ichor. Below are the commands for running the tests both with and without code coverage.

*Note: BEFORE running the tests, you should run the `make clean` command to ensure that all object files are rebuilt with the correct flags for code coverage if you plan to generate a coverage report. This only needs to be run once before running the test suites since we want the coverage to build off one another.*

#### Build and Run Unit Tests Without Coverage
```bash
# Build and execute all tests
make test_all

# Individual make commands
make test_common
make test_erv
make test_ichor
```

#### Build and Run Unit Tests With Coverage
```bash
# Build and execute all tests and generate junit xml reports for each test file
make test_all_report

# Individual make commands which generate junit xml reports for each test file
make test_common_report
make test_erv_report
make test_ichor_report
```

#### Generating Code Coverage Report
A local version of the code coverage report can be generated using `lcov` and `genhtml`. If you are using the dev container, these tools should already be included. If not, `lcov` and `genhtml` can be downloaded with the following command.

```bash
apt-get install lcov
```
*Note: If you have any trouble please refer to the [lcov README](https://github.com/linux-test-project/lcov) for instructions on another method of installation.*

To generate the html coverage report and save it to the `coverage/` directory, run the following commands from the root of the repository:
```bash
# With make
make cov_report

# Without make
lcov --capture --directory build/obj/ --output-file coverage.info --exclude '*/12/*' --exclude '/usr/include/*' --exclude '*/tests/*'
genhtml coverage.info --output-directory coverage
```
