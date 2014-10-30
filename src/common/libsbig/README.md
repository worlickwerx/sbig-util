### FITS Header Example

Moving towards full support of the SBIG standard header extensions.
Not quite there yet - missing the following mandatory headers:

```
CBLACK, CWHITE, PEDESTAL, DATAMAX, SWCREATE, SWMODIFY
```

And the following optional headers:
```
IMAGETYP, SET-TEMP, TRAKTIME, OBJECTRA, OBJECTDEC, CENTAZ,
CENTALT, SITELAT, SITELONG
```

### Example header from sbig-snap
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
DATE    = '2014-10-30T21:07:31' / GMT date when this file created               
DATE-OBS= '2014-10-30T21:07:37' / GMT start of exposure                         
EXPTIME =                   1. / Exposure in seconds                            
CCD-TEMP=     26.2257777130536 / CCD temp in degress C                          
OBJECT  = 'm33     '                                                            
TELESCOP= 'Nikkor-Q'                                                            
FILTER  = 'Astrodon Tru-balance E-series NIR blocked L' / Optical filter name   
OBSERVER= 'Jim Garlick'                                                         
INSTRUME= 'SBIG ST-8 Dual CCD Camera' / Camera Model                            
XBINNING=                    1 / Horizontal binning factor                      
YBINNING=                    1 / Vertical binning factor                        
XPIXSZ  =                   9. / Pixel width in microns                         
YPIXSZ  =                   9. / Pixel height in microns                        
EGAIN   =                 2.78 / Electrons per ADU                              
XORGSUBF=                    0 / Subframe origin x_pos                          
YORGSUBF=                    0 / Subframe origin y_pos                          
RESMODE =                    0 / Resolution mode                                
SBSTDVER= 'SBFITSEXT Version 1.0' / SBIG FITS extensions ver                    
SNAPSHOT=                    1 / Number images coadded                          
FOCALLEN=                 135. / Focal length in mm                             
APTDIA  =                  33. / Aperture diameter in mm                        
APTAREA =               103.62 / Aperture area in sq-mm                         
END                                                                             
```
