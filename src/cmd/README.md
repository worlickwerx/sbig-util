### Example config file

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
