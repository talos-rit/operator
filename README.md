# Scorbot ER 4PC Driver
This library provides classes and functions to control the Scorbot ER 4PC robotic arm as part of the RIT Talos project. Since our team does not have a functioning controller for the ER 4PC, this library is intended to allow us to interface with the robot using a Raspberry Pi without the need for the original controller. It is intended to be used in conjunction with the [RIT Talos Operator](https://github.com/talos-rit/operator) project.

*Note: This driver does not currently fully replicate all functionalities of the original Scorbot controller. Some features may be limited or unavailable.*

## Compilation and Development
To compile the project, please refer to the instructions provided in the [RIT Talos Operator README](https://github.com/talos-rit/operator/blob/main/README.md#initial-setup) starting with the Initial Setup if you have not done so already. The operator repo is a parent repository that includes this driver as a submodule. If you are on `macOS` or `Windows`, please refer to [Additional Steps for macOS/Windows](https://github.com/talos-rit/operator/blob/main/README.md) in the RIT Talos Operator README. You can use the same steps to create a Dev Container for development of this project.

