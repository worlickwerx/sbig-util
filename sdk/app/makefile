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

ARCHDIR    = $(PWD)/../libs/$(shell arch | sed -e 's/armv6l/arm_v6/' \
                                               -e 's/armv7l/arm_v7/' \
                                               -e 's/armv8l/arm_v8/')
INCLUDE    = -I/usr/include/libusb-1.0/
LIBRARY    = -Wl,-rpath,$(ARCHDIR) -L$(ARCHDIR)


APP_FLAGS  = -O2 $(INCLUDE) -Wall
####################################################################################
all: $(APP_NAME)
####################################################################################
$(APP_NAME): $(APP_OBJ)
	  g++ $(APP_FLAGS) -o $(APP_DEST)$(APP_NAME) $(APP_OBJ) $(LIBRARY) -lsbigudrv -lcfitsio -lm -lusb-1.0
	  chmod a+x $(APP_DEST)$(APP_NAME)

testmain.o : testmain.cpp csbigcam.h csbigimg.h
	  g++ $(APP_FLAGS) -o testmain.o -c testmain.cpp

csbigcam.o : csbigcam.cpp csbigcam.h csbigimg.h
	  g++ $(APP_FLAGS) -o csbigcam.o -c csbigcam.cpp

csbigimg.o : csbigimg.cpp csbigimg.h
	  g++ $(APP_FLAGS) -o csbigimg.o -c csbigimg.cpp
####################################################################################
clean:
		rm -f *.o testapp
####################################################################################
