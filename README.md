### sbig-util

_Under contstruction: this project was just begin on 14 Sept 2014,
and so far there is no intelligent life here!  Move along..._

sbig-util is a set of Linux command line tools for controlling cameras
from the Santa Barbara Instrument group.

The goal is to develop a set of command line tools that can directly
control a camera and filter wheel in the field during an astrophotography
session.

sbig-util includes a copy of the SBIG Linux Development kit and firmware
as a convenience.  You may configure sbig-util to use either this copy,
or another external one.  These instructions presume that you will use
the internal one.

### Supported Platforms

We are limited to architectures for which SBIG pre-compiled the
libraries in the SDK.  For the 2014-01-10 release, this includes:
* 32-bit ARM
* 32-bit x86
* 64-bit x86.
Further, SBIG claims they tested this release on Debian 6.0.6 and
Ubuntu 12.04.  

My testing thus far is on a BeagleBone black (ARM) running Debian 7.0
(Wheezy), with an SBIG ST-8XME camera and CFW-10 filter wheel connected
to the camera through i2c.

### Prerequisites

You will need to install libusb version 1.0.x and fxload.
On Debian Wheezy use:
```
sudo apt-get install fxload
sudo apt-get install libusb-1.0-0-dev

### USB firmware download

The devkit includes a set of udev rules and .hex files for USB cameras.
When udev detects an SBIG USB device, the udev rules invoke fxload to
download the appropriate .hex file into the camera.  For this to work
you must copy the udev rules and firmware files to the following locations:
```
sudo cp sdk/LinuxDevKit.2014-01-10/udev/51-sbig-debian.rules /etc/udev/rules.d
sudo cp sdk/LinuxDevKit.2014-01-10/firmware/* /lib/firmware/
```
You may need to then poke udev with
```
udevadm control --reload-rules
```
When my camera is properly initialized, the red LED on the back blinks
for a few seconds, then the cooling fan comes on.

