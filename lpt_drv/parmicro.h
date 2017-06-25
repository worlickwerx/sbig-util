/*

	parmicro.h - Contains the shorterface routines for the parmicro module
				 of the parallel driver dealing with the medium and
				 low level microcontroller routines.

*/

#ifndef _PARMICRO_
#define _PARMICRO_

/*

	External Includes

*/
#ifndef _LPARDRV_
#include "lpardrv.h"
#endif

/*

	Constants

*/
#define ETH_FIFO_SIZE		(4096)		// size of Fifo in camera
#define USB_FIFO_SIZE		(4*4096)	// size of Fifo in in driver for USB cameras
#define MAX_FIFO_SIZE		(ETH_FIFO_SIZE > USB_FIFO_SIZE ? ETH_FIFO_SIZE : USB_FIFO_SIZE)
#define USB_PIPELINE_SIZE	4			// size of AD pipeline register

/*

	Config Flags - Read from EEPROM

*/
#define CONFIG_LOW_TRACKER_IS_237		1
#define CONFIG_IMAGER_IS_311			2
#define CONFIG_LOW_ALT_2K_TRANSFER		4
#define CONFIG_LOW_STF_MOTOR_PHASE		0xC0

/*

	External Variables

*/
extern short baseAddress;

/*

	Type Defines

*/
typedef enum { MC_START_EXPOSURE=0, MC_END_EXPOSURE, MC_REGULATE_TEMP,
	MC_TEMP_STATUS, MC_RELAY, MC_PULSE, MC_GET_VERSION, MC_EEPROM,
	MC_MISC_CONTROL, MC_STATUS, MC_SYSTEM_TEST, MC_TX_BYTES,
	MC_CONTROL_CCD, MC_COMMAND_13, MC_SYSTEM,
	MC_READOUT, MC_REGULATE_TEMP2=128+MC_REGULATE_TEMP } MICRO_COMMAND;
typedef enum { SHUTTER_ASIS, OPEN_SHUTTER, CLOSE_SHUTTER, OPEN_EXT_SHUTTER = 0x20,
	CLOSE_EXT_SHUTTER=0x40 } SHUTTER_CONTROL;
typedef enum { SHUTTER_OPEN, SHUTTER_CLOSED, SHUTTER_OPENING,
	SHUTTER_CLOSING } SHUTTER_STATUS;
typedef enum { CCD_IDLE, PRE_SHUTTER, INTEGRTAING, POST_SHUTTER } CCD_STATE;
typedef enum {RS_DIG_ROW, RS_DLP_ROW, RS_DL_ROW, RS_DLP_ROWS,
	RS_DUMP_FIFO, RS_DL_SETUP, RS_DUMP_ROWS, RS_CLEAR_CCD,
	RS_SET_VDD, RS_WRITE_AD, RS_DLPP_ROWS, RS_END_READOUT } READOUT_SUBCOMMAND;
typedef enum { SYS_READ_INT, SYS_WRITE_INT, SYS_READ_EXT, SYS_WRITE_EXT,
	SYS_GET_ROM_SUM, SYS_WRITE_SFR, SYS_INIT_GA, SYS_SET_MOTOR_PHASE } SYSTEM_SUBCOMMAND;
typedef enum {LINK_NONE, LINK_LPT, LINK_USB, LINK_ETH, LINK_ETH_USB } LINK_TYPE;
typedef enum {SMP_PHASE0, SMP_PHASE1, SMP_PHASE2, SMP_PHASE3, SMP_QUERY } SET_MOTOR_PHASE_CMD;

typedef struct {
	LINK_TYPE linkType;
	SBIG_DEVICE_TYPE deviceType;
	MY_LOGICAL open;
	unsigned long comTotal;
	unsigned long comFailed;
	unsigned long comPassed;
} LINK_INFO;

typedef struct {
    TEMPERATURE_REGULATION regulation;
    unsigned short ccdSetpoint;
	unsigned short preload;
	} MicroTemperatureRegulationParams;
typedef struct {
    MY_LOGICAL freezeTE;
    MY_LOGICAL lowerPGain;
	} MicroTemperatureSpecialParams;
typedef struct {
	unsigned char address;
	unsigned char data;
	MY_LOGICAL write;
	unsigned char deviceAddress;
	} EEPROMParams;
typedef struct {
	unsigned char data;
	} EEPROMResults;
typedef struct {
	unsigned short firmwareVersion;
	unsigned short cameraID;
	} GetVersionResults;
typedef struct {
    CCD_STATE imagingState, trackingState;
    SHUTTER_STATUS shutterStatus;
    LED_STATE ledStatus;
	MY_LOGICAL xPlusActive, xMinusActive;
	MY_LOGICAL yPlusActive, yMinusActive;
	MY_LOGICAL fanEnabled;
	MY_LOGICAL CFW6Active;
	MY_LOGICAL CFW6Input;
	MY_LOGICAL extShutterClosed;
	unsigned char shutterEdge;
    FILTER_STATE filterState;
    AD_SIZE adSize;
    FILTER_TYPE filterType;
	} StatusResults;
typedef struct {
	unsigned short checksum;
	unsigned char version;
	unsigned char model;
	unsigned char abgType;
	unsigned char badColumns;
	unsigned short trackingOffset;
	unsigned short trackingGain;
	unsigned short imagingOffset;
	unsigned short imagingGain;
	unsigned short columns[3];
	unsigned short configWord;
	unsigned char serialNumber[10];
	} EEPROMContents;
typedef struct {
	unsigned short checksum;
	unsigned char version;
	unsigned char model;
	unsigned char abgType;
	unsigned char subModel;
	unsigned short trackingOffset;
	unsigned short trackingGain;
	unsigned short imagingOffset;
	unsigned short imagingGain;
	unsigned short spare0, spare1, spare2;
	unsigned short configWord;
	unsigned char serialNumber[10];
	} AltEEPROMContents;
typedef struct {
	unsigned char subCommand;
	} ReadoutParams;
typedef struct {
	unsigned char subCommand;
	unsigned char /* CCD_REQUEST */ ccd;
	unsigned short columns;
	unsigned short rows;
	} ReadoutAreaParams;
typedef struct {
	unsigned char subCommand;
	unsigned char /* CCD_REQUEST */ ccd;
	unsigned char horzBin;
	unsigned char vertBin;
	unsigned short left;
	unsigned short right;
} ReadoutSetupParams;
typedef struct {
	unsigned char subCommand;
	unsigned char /* CCD_REQUEST */ ccd;
	unsigned char vertBin;
	unsigned short rowWidth;
	unsigned short rows;
	unsigned char vToHMask;
} ReadoutDumpParams;
typedef struct {
	unsigned char subCommand;
	unsigned char raiseIt;
} ReadoutVddParams;
typedef struct {
	unsigned char subCommand;
	unsigned char vddWasLow;
} ReadoutVddResults;
typedef struct {
	unsigned char subCommand;
	unsigned char byte1;
	unsigned char byte2;
} ReadoutWriteADParams;
typedef struct {
	unsigned char subCommand;
	unsigned char ccd;
} ReadoutEndReadoutParams;
typedef struct {
	short rowWidth;			/* rows to download from camera */
	short reqRowWidth;		/* pixels asked for by user */
	short rowsPerPass;		/* rows to to per USB transactions */
	short rowsInFifo;		/* no. rows in fifo */
	short bPipelineFull;	/* TRUE when already pipelined */
} FifoInfo;
typedef struct {
	unsigned char subCommand;
	unsigned char len;
	short destAddress;
	unsigned char data[8];
} SystemMemoryParams;
typedef struct {
	unsigned char subCommand;
	unsigned char len;
	short destAddress;
	unsigned char data[58];
} SystemMemoryParams1;
typedef struct {
	unsigned char data[8];
} SystemMemoryResults;
typedef struct {
	unsigned short sum;
} GetROMSumResults;
typedef struct {
	unsigned char subCommand;
	unsigned char address;
	unsigned char andMask;
	unsigned char orMask;
} SystemWriteSFRParams;
typedef struct {
	unsigned char data;
} SystemWriteSFRResults;
typedef struct {
	unsigned char subCommand;
	unsigned char phaseCmd;
} SystemSetMotorPhaseParams;
typedef struct {
	unsigned char motorPhase;
} SystemSetMotorPhaseResults;

/*

	Externally accessable micro routines

*/
extern PAR_ERROR MicroCommand(MICRO_COMMAND command, CAMERA_TYPE camera,
	void *txDataPtr, void *rxDataPtr);
extern unsigned short CalculateEEPROMChecksum(EEPROMContents *eePtr);
extern PAR_ERROR GetEEPROM(CAMERA_TYPE camera, EEPROMContents *eePtr);
extern PAR_ERROR PutEEPROM(CAMERA_TYPE camera, EEPROMContents *eePtr);
PAR_ERROR MicroInitPixelReadout(CAMERA_TYPE cameraID, CCD_REQUEST ccd,
								short left, short noPixels, 
								short right, short height, short horzBin, short vertBin);
extern PAR_ERROR MicroGetPixels(CAMERA_TYPE cameraID, unsigned short *dest);
extern PAR_ERROR BitIOCommand(BitIOParams *pParams, BitIOResults *pResults);

/*

	Windows DLL Exports

*/
#if TARGET == ENV_WIN
 extern PAR_ERROR __stdcall MicroCommand2(MICRO_COMMAND command, CAMERA_TYPE camera,
	void *txDataPtr, void *rxDataPtr);
 extern PAR_ERROR __stdcall GetEEPROM2(CAMERA_TYPE camera, EEPROMContents *eePtr);
 extern PAR_ERROR __stdcall PutEEPROM2(CAMERA_TYPE camera, EEPROMContents *eePtr);
#endif

#endif
