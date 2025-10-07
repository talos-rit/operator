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
5. A file explorer window will open. Select the cloned repository folder.
6. When prompted to Add Dev Container Configuration Files select `Add configuration to workspace` and then the `C++` dev container for developing C++ applications on Linux. 
7. For the OS, select `debian-11`. 
8. For the CMake version select `none`, and then click ok for the rest of the prompts. The dev container should start up and you should be ready for development.
9. To compile the operator, open a terminal in VS Code and run the compilation commands mentioned above in [Compilation](#compilation).

