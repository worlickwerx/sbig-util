### EthSim2

Santa Barbara Instrument Group  
EthSim2 Read Me file  
February 25, 2010  

EthSim2 is a Windows GUI program that simulates various model SBIG cameras over Ethernet. It is an invaluable tool when developing software to support SBIG cameras. Here are some helpful notes:

* EthSim2 can simulate any Parallel or USB SBIG camera currently in production. You select which camera to simulate with the Camera Type pop-up combo box.
* You can run EthSim2 on your development computer or a remote computer over a LAN. You connect to it with the `CC_OPEN_DEVICE` command by specifying device = `DEV_ETH` and the IP Address of the server which is shown in `EthSm2`.
* EthSim2 can produce three types of images, controlled by the Image Type combo pop-up box:
  * When Image Type is set to `Noise` or for dark frames EthSim2 generates random pixel data with an RMS noise of approximately 6 ADU.
  * When Image Type is set to `Stars` EthSim2 generates images with generated stars. The Stars image is 200 x 200 pixels and will repeat for larger areas.
  * When Image Type is set to `Photo`, EthSim2 will send data from the SBIG uncompressed image named `SIMIMAGE.SBIG` that it finds in the same directory as EthSim2. We supply a sample ST-7 image (765 x 510) but you can use an uncompressed SBIG format image. Again, the image will repeat for larger areas.
* EthSim2 will move the Stars image in response to Relay or AO commands, moving 5 pixels per second for Relay moves and +/- 4 pixels for AO commands.
* EthSim2 will also simulate internal and external color filter wheels like the CFW5, CFW-8 and the STLs CFW-L.

2008-01-15  Released version 2.03 with the following additions:
* Fixed a bug that would cause binned partial frames to be misaligned.
* Added options to specify the type of tracking CCD (211 or 237) and whether simulated camera simulates an external guide head.
* Motion of the External Guide Head for Relay and AO motion is at 45 degrees to internal CCDs.
* Made the random motion of the 2nd brightest start in the field of view different that the other stars so could simulate differential guiding.
* Added support for the ST-4000XCM camera.
* Now displays the IP address in the window title.

2010-02-25  Released version 2.04 Build 3 with the following additions:
* Added support for the STX Cameras.
* Fixed a bug where were hogging CPU time.
* Fixed a bug where werent stopping the Server on exit that could cause kernel errors.
* Added the Ethernet Echo Server so CCDOps and the Driver Library can discover the IP address.
* When serving a TC-237 Guider center the star pattern and expand the spacing to 100 pixels so only one pattern of 5 stars shows.
* Added support for the ST-8300 and ST-8300C cameras.

We hope you find this tool useful. If you have any comments or bug reports send an email to <matto @ sbig . com>.
