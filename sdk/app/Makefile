####################################################################################
# The Makefile of SBIG Linux USB testing application.
####################################################################################
# use:
# make clean -f Makefile
# make -f Makefile

# Note: Linker uses shared libraries, so you should copy *.so library to the 
# /usr/local/lib/ directory or soft link to it and update LD_LIBRARY_PATH. 
####################################################################################
APP_DEST   = ./
APP_NAME   = testapp
APP_SRC    = testmain.cpp csbigcam.cpp csbigimg.cpp
APP_OBJ    = testmain.o   csbigcam.o   csbigimg.o

# use if FITS stuff is installed
#INCLUDE    = -I .ls -I /usr/include/libusb-1.0/ -I /data/fits/cfitsio
#LIBRARY    = -L /usr/local/lib -L /data/fits/cfitsio/lib/

INCLUDE    = -I .ls -I /usr/include/libusb-1.0/
LIBRARY    = -L /usr/local/lib

APP_FLAGS  = -O2 $(INCLUDE) $(LIBRARY) -Wall
####################################################################################
all: $(APP_NAME)
####################################################################################
# use if FITS stuff is installed
#$(APP_NAME): $(APP_OBJ)
#	  g++ $(APP_FLAGS) -o $(APP_DEST)$(APP_NAME) $(APP_OBJ) -lsbigudrv -lcfitsio -lm -lusb-1.0
#	  chmod a+x $(APP_DEST)$(APP_NAME)

$(APP_NAME): $(APP_OBJ)
	  g++ $(APP_FLAGS) -o $(APP_DEST)$(APP_NAME) $(APP_OBJ) -lsbigudrv -lm -lusb-1.0
	  chmod a+x $(APP_DEST)$(APP_NAME)

testmain.o : testmain.cpp csbigcam.h csbigimg.h
	  g++ $(APP_FLAGS) -o testmain.o -c testmain.cpp

csbigcam.o : csbigcam.cpp csbigcam.h csbigimg.h
	  g++ $(APP_FLAGS) -o csbigcam.o -c csbigcam.cpp

csbigimg.o : csbigimg.cpp csbigimg.h
	  g++ $(APP_FLAGS) -o csbigimg.o -c csbigimg.cpp
####################################################################################
clean:
		rm -f *.o ${APP_NAME} 
####################################################################################
