# Basic information
The RPI is a Raspberry Pi 4B (4GB), running a customized image of Raspbian Lite 64 OS (Rasbian being a fork of Debian, meant specifically for Raspberry Pis). This custom image is special because it is running the PREEMPT_RT kernel patch, allowing the Linux scheduler to operate like an RTOS scheduler, including full preemption of tasks.

Its purpose as part of the Talos project is to act as a dedicated piece of hardware to run the Operator application. It's compatible with both the ER V version, as well as the still experimental Ichor verison (where the RPI will be managing the motors and sensors directly).

# User information
- username: pi
- password: raspberry

# Connection options
For basic command and control of the RPI, a linux console can be used, either from a serial TTL cable (the 4 pin USB adapter), or through SSH over a network.


## TTL Instructions
Unfortunately, every OS has their own way of defining port names, so check how your OS does it. The connection runs at 115200 baud
### Unix
Unix machines tend to use screen for TTL devices: "sudo screen <device-name> 115200"

- Ubuntu: Typically, /dev/ttyUSBn, where n is a number assigned by your OS
- Mac: TBD

### Windows
Windows has to be special, so most of the time a program like PuTTY is used to connect. Principles are still the same, set PuTTY to use serial instead of SSH, and set the baud to 115200
- Windows: COMn, where n is a number assigned by your OS


When it comes to running Talos (commander and operator programs), it requries a network socket, so the TTL cable won't work.

Currently, due to RIT's heavily locked down network, it's hard to ssh from one host to another unless specifically setup by ITS (good luck with that). As a simple development work around, the RPI has been configured to act as a hotspot, so the machine running the commander can connect to the RPI running the operator program directly, allowing ssh and the network socket necessary for Talos.

## Hotspot Instructions
If you can't connect to the RPI through the network, and need to run Talos, the best option is to directly connect using the Hotspot, so long as you can live without internet access in the meantime.

### Wifi details
- SSID: Talos
- Password: Operator

### SSH/Domain information
Once connected, the hotspot DNS is configured such that you can use the domain "pi@operator.talos" instead of "pi@192.168.0.10" (at present, the RPI ip address is statically set to 192.168.0.10 when operating as a hotspot).



