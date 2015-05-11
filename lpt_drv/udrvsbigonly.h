/*

	UDRVSBIGONLY.H - Contains the typedefs and prototypes
					 of SBIG only code for the driver.

*/
#ifndef _UDRVSBIGONLY_
#define _UDRVSBIGONLY_

typedef enum { STL_1001_CAMERA, STL_11K_CAMERA, STL_4K_CAMERA,
  STL_5K_CAMERA, STL_6K_CAMERA, STL_1301_CAMERA, STL_NEXT_CAMERA } STL_CAMERA_TYPE;

#define DO_VERSION_1		1		// initial version
#define DO_VERSION_2		2		// added something
#define DO_VERSION_3		3		// added doReportShutterError
#define DO_VERSION_4		4		// added doUseImagerDSM
#define DO_VERSION_5		5		// removed doUseImageDSM, doUSBGARev
#define DO_CURRENT_VERSION	DO_VERSION_5
typedef enum {DO_PER_DRIVER, DO_TRUE, DO_FALSE } DRIVER_OPTION_BOOL;
typedef struct {
	short doVersion;
	short doSize;
	short doBiasSubtraction;		/* TRUE when bias subtraction enabled */
	short doVddLowForIntegration;	/* TRUE when lower Vdd for integration */
	short doAutoFreezeTE;			/* TRUE when auto freeze TE cooler for readout */
	short doReportShutterErrors;	/* TRUE when shutter errors reported */
	short doUSBFifoSize;			/* Size of FIFO in USB cameras */
} UDRV_OPTIONS;

/*  CFW-L Constants */
#define CFWL_DATA_REG_LENGTH	16		/* bytes in CFW-L Status Data */
#define CFWL_CAL_LENGTH			1500	/* bytes in CFW-L Cala Data */

#endif
