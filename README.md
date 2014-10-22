_Under contstruction:_ this project was just begin on 14 Sept 2014,
and so far there is no intelligent life here!  Move along...

### sbig-util

sbig-util is a set of Linux command line tools for controlling cameras
from the Santa Barbara Instrument group.  The goal is to develop a simple
tool that can directly control a camera and filter wheel in the field
during an astrophotography session.  Perhaps our tool can be a building
block for more complex, process-oriented software, or serve as another
demo for using the SBIG SDK.

sbig-util includes a copy of the SBIG Linux Development Kit
as a convenience.  You may configure sbig-util to use either this copy,
or another external one.  These instructions presume that you will use
the internal one.  The SDK includes firmware, udev rules, documentation,
the SBIG universal (all user space) driver, and some test code.

### Supported Platforms

We are limited to architectures for which SBIG pre-compiled the
libraries in the SDK.  As of this writing, this includes:
* 32-bit ARM (compiled on Raspberry Pi, version unknown)
* 32-bit x86 (compiled on Ubuntu 14.04)
* 64-bit x86 (compiled on Ubuntu 14.04)

My testing thus far is on a BeagleBone black (ARM) running Debian
7.0 (Wheezy), with an SBIG ST-8XME USB camera and CFW-10 filter wheel.

### Prerequisites
You will need to install libusb version 1.0.x and fxload,
as well as cfitsio for the SBIG test application.  On Debian Wheezy use:
```
sudo apt-get install fxload
sudo apt-get install libusb-1.0-0-dev
sudo apt-get install cfitsio-dev
```

### USB firmware download

The devkit includes a set of udev rules and .hex files for USB cameras.
When udev detects an SBIG USB device, the udev rules invoke fxload to
download the appropriate .hex file into the camera.  For this to work
you must copy the udev rules and firmware files to the following locations:
```
sudo cp sdk/udev/51-sbig-debian.rules /etc/udev/rules.d
sudo cp sdk/firmware/* /lib/firmware/
```
You may need to then poke udev with
```
sudo udevadm control --reload-rules
```
When my camera is properly initialized, the red LED on the back blinks
for about a second, then the cooling fan comes on.

### Building and running the SDK test code

To validate that everything is installed properly thus far,
you can run SBIG's test code.
```
cd sdk/app
make
./testapp
```
This displays a help message:
```
./testapp filePath fileType imgCount imgType expTime rm top left width heigh fr dcm
```
For example, to snap an image from the camera and store it in /tmp, run
```
./testapp /tmp/ SBIG 1 LF 0.01 1x1 0 0 0 0 1 1
```
