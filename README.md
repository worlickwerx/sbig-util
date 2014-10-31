### sbig-util

sbig-util is a set of Linux command line tools for controlling cameras
from the Santa Barbara Instrument group.  The goal is to develop a simple
tool that can directly control a camera and filter wheel in the field
during an astrophotography session, and be part of a workflow that
includes other AIPS tools such as [ds9](http://ds9.si.edu).

sbig-util includes a copy of the SBIG Linux Development Kit
as a convenience.  You may configure sbig-util to use either this copy,
or another external one.  These instructions presume that you will use
the internal one.  The SDK includes firmware, udev rules, documentation,
the SBIG universal (all user space) driver, and some test code.

### Supported Platforms

We are limited to architectures for which SBIG pre-compiled the
libraries in the SDK.  As of this writing, sbig-util includes the
SDK drop dated 2014-10-26T18-10 built for
* 32-bit ARM (v6, v7, v8)
* 32-bit x86
* 64-bit x86

As I understand it all builds were on Debian 7 derived distros
(Raspbian for arm v6, Ubuntu 14.04 for the rest).  My testing thus
far is on a BeagleBone black (ARM) running Debian 7.0 (Wheezy),
with an SBIG ST-8XME USB camera and CFW-10 filter wheel.

### Prerequisites

You will need to install libusb version 1.0.x, fxload, and cfitsio.
On Debian Wheezy use:
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
you can build and run SBIG's test code.
```
cd sdk/app
make
```
The usage is:
```
./testapp filePath fileType imgCount imgType expTime rm top left width heigh fr dcm

fileType: SBIG or FITS
imgCount: integer number of images to snap
imgType:  LF=light field or DF=dark field
expTime:  floating point exposure time in seconds
rm:       readout mode 1x1, 2x2, 3x3 
top, left, width, height:
          image size (set all to zero for full size)
fr:       1=enable fast readout (at cost of noise)
dcm:      1=enable dual channel mode
```
For example, to snap an image from the camera and store it in /tmp, run
```
./testapp /tmp/ FITS 1 LF 0.01 1x1 0 0 0 0 1 1
```

### Building sbig-util

To build sbig-util, run
```
./autogen.sh
./configure
make
```

### Running sbig-info

You can get your camera connected and run
```
cd src/cmd
./sbig info ccd imaging
```
which should display information about your camera.  For example
my ST-8 looks like this:
```
sbig-info: firmware-version: 2.42
sbig-info: camera-type:      ST-8
sbig-info: name:             SBIG ST-8 Dual CCD Camera
sbig-info: readout-modes:
sbig-info:  0: 1530 x 1020 2.78 e-/ADU 9.00 x 9.00 microns
sbig-info:  1:  765 x 510  2.78 e-/ADU 18.00 x 18.00 microns
sbig-info:  2:  510 x 340  2.78 e-/ADU 27.00 x 27.00 microns
sbig-info:  3: 1530 x 0    2.78 e-/ADU 9.00 x 9.00 microns
sbig-info:  4:  765 x 0    2.78 e-/ADU 18.00 x 9.00 microns
sbig-info:  5:  510 x 0    2.78 e-/ADU 27.00 x 9.00 microns
sbig-info:  6: 1530 x 1020 2.78 e-/ADU 9.00 x 9.00 microns
sbig-info:  7:  765 x 510  2.78 e-/ADU 18.00 x 18.00 microns
sbig-info:  8:  510 x 340  2.78 e-/ADU 27.00 x 27.00 microns
sbig-info:  9:  170 x 113  2.78 e-/ADU 81.00 x 81.00 microns
sbig-info: bad columns:       0
sbig-info: ABG:               no
sbig-info: serial-number:     05121350
sbig-info: ccd-type:          full frame
sbig-info: electronic-shutter:no
sbig-info: remote-guide-port: no
sbig-info: biorad-tdi-mode:   no
sbig-info: AO8-detected:      no
sbig-info: frame-buffer:      no
sbig-info: use-startexp2:     no
```

`sbig info` can also query information about the tracking ccd,
cfw (filter wheel), cooler, driver, and calculate angular field of view
for a given focal length telescope.

### Running sbig-cooler

You can enable the TE cooler for a setpoint in degrees Celcius,
and watch it stablize with the following commands:
```
./sbig cooler on -30
./sbig info cooler
```

### Running sbig-snap

sbig-snap generates FITS files and needs you to supply some static
information that it will enter into the FITS header.  The config file
should be placed in `$HOME/.sbig/config.ini`.  Here is an example:
```
[system]
device = USB1               ; USB1 thru USB8, ...
imagedir = /tmp             ; FITS files will be created here
;sbigudrv = /usr/local/lib/libsbigudrv.so

[cfw]
slot1 = Astrodon Tru-balance E-series NIR blocked R
slot2 = Astrodon Tru-balance E-series NIR blocked G
slot3 = Astrodon Tru-balance E-series NIR blocked B
slot4 = Astrodon Tru-balance E-series NIR blocked L

[config]
observer = Jim Garlick     ; Telescope operator
;filter = cfw               ; Filter selected by CFW
filter = Astrodon Tru-balance E-series NIR blocked L ; Fixed filter
telescope = Nikkor-Q       ; Telescope (CLA-7 Nikon adapter, stopped at f/4)
focal_length = 135         ; Focal length of the telescope in mm
aperture_diameter = 33     ; Aperture diameter of the telescope in mm
aperture_area = 854.86     ; Aperture area in sq-mm (correct for obstruction)

[site]
name = Carnelian Bay, CA
latitude = +39:13:36.6636   ; Latitude, degrees
longitude = +120:04:54.6924 ; Longitude, degrees W. of zero
elevation = 1928            ; Elevation, meters
```

To take a picture, e.g. a 30s exposure of M31:
```
./sbig snap --object M31 -t 30
```

Currently sbig-snap only takes "autodark" images.  That is, it takes
a dark frame and a light frame and writes the result of subtracting
the dark from the light.

sbig-snap has various options to play with.  Run `sbig snap -h` to list them.

### FITS headers

sbig-util is moving towards full support of SBIG's FITS header
extensions described in
[this document](http://archive.sbig.com/pdffiles/SBFITSEXT_1r0.pdf).
These mandatory headers are not yet being generated:

```
CBLACK, CWHITE, PEDESTAL, DATAMAX, SWCREATE, SWMODIFY
```

And the following optional headers:
```
IMAGETYP, SET-TEMP, TRAKTIME, OBJECTRA, OBJECTDEC, CENTAZ, CENTALT
```
Here is an example header:
```
SIMPLE  =                    T / file does conform to FITS standard             
BITPIX  =                   16 / number of bits per data pixel                  
NAXIS   =                    2 / number of data axes                            
NAXIS1  =                 1530 / length of data axis 1                          
NAXIS2  =                 1020 / length of data axis 2                          
EXTEND  =                    T / FITS dataset may contain extensions            
COMMENT   FITS (Flexible Image Transport System) format is defined in 'Astronomy
COMMENT   and Astrophysics', volume 376, page 359; bibcode: 2001A&A...376..359H 
BZERO   =                32768 / offset data range to that of unsigned short    
BSCALE  =                    1 / default scaling factor                         
COMMENT = 'SBIG FITS header format per:'                                        
COMMENT = ' http://www.sbig.com/pdffiles/SBFITSEXT_1r0.pdf'                     
SBSTDVER= 'SBFITSEXT Version 1.0' / SBIG FITS extensions ver                    
DATE    = '2014-10-30T22:01:06' / GMT date when this file created               
DATE-OBS= '2014-10-30T22:01:13' / GMT start of exposure                         
EXPTIME =                   1. / Exposure in seconds                            
CCD-TEMP=      26.827323877487 / CCD temp in degress C                          
SITENAME= 'Carnelian Bay, CA'  / Site name                                      
SITEELEV=                1928. / Site elevation in meters                       
SITELAT = '+39:13:36.6636'     / Site latitude in degrees                       
SITELONG= '+120:04:54.6924'    / Site longitude in degrees west of zero         
OBJECT  = 'M42     '           / Name of object imaged                          
TELESCOP= 'Nikkor-Q'           / Telescope model                                
FILTER  = 'Astrodon Tru-balance E-series NIR blocked L' / Optical filter name   
OBSERVER= 'Jim Garlick'        / Telescope operator                             
INSTRUME= 'SBIG ST-8 Dual CCD Camera' / Camera Model                            
XBINNING=                    1 / Horizontal binning factor                      
YBINNING=                    1 / Vertical binning factor                        
XPIXSZ  =                   9. / Pixel width in microns                         
YPIXSZ  =                   9. / Pixel height in microns                        
EGAIN   =                 2.78 / Electrons per ADU                              
XORGSUBF=                    0 / Subframe origin x_pos                          
YORGSUBF=                    0 / Subframe origin y_pos                          
RESMODE =                    0 / Resolution mode                                
SNAPSHOT=                    1 / Number images coadded                          
FOCALLEN=                 135. / Focal length in mm                             
APTDIA  =                  33. / Aperture diameter in mm                        
APTAREA =               854.86 / Aperture area in sq-mm                         
END                                                                             
```
