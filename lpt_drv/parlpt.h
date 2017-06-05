/*

	parlpt.h - contains the prototypes for the parallel specific
			   code.

*/
#ifndef _PARLPT_
#define _PARLPT_

#define HSI					0x10	// hand shake input bit
#define CIP					0x10	// conversion in progress bit
#define CAN                 0x18	// CAN response from Micro
#define NAK                 0x15	// NAK response from Micro
#define ACK                 0x06	// ACK response from Micro

#define NIBBLE_TIMEOUT		5		// ticks between chars received from camera
									// increased from 2 to 5 for TCE
#define CONVERSION_DELAY	50000	// queries (approx. 1us each) before
									// A/D must signal not busy
#define VCLOCK_DELAY		10		// delays between vertical clocks
									//  on the imaging CCD
#define ST1K_VCLOCK_X		10		// multiplier for VClock an ST1K
#define CLEAR_BLOCK     	10		// number of pixels cleared in a block
#define DUMP_RATIO			5		// after every N vertical clocks
									//  do a full horizontal clock on
#define BIAS_WIDTH			128     // number of pixels to average for bias
#define IDLE_STATE_DELAY	3		// time to force idle at start of packet


typedef enum { TRACKING_CLOCKS=0x00, IMAGING_CLOCKS=0x10, MICRO_OUT=0x20,
	CONTROL_OUT=0x30, READOUT_CONTROL=0x00, DEVICE_SELECT=0x60 } OUTPUT_REGISTER;
typedef enum { AD0=0x00, AD1=0x10, AD2=0x20, AD3_MDI=0x30 } INPUT_REGISTER;
typedef enum { HSO=0x01, MICRO_SELECT=0x02, IMAGING_SELECT=0x00,
	TRACKING_SELECT=0x04, MICRO_SYNC=0x04, AD_TRIGGER=0x08 } CONTROL_BITS;

typedef enum { V1_H=1, V2_H=2, TRG_H=4, IABG_M=8 } IMAGING_CLOCK_BITS;
typedef enum { P1V_H=1, P2V_H=2, P1H_L=8 } KI_CLOCK_BITS;
typedef enum { IAG_H=1, TABG_M=2, BIN=4, CLR=8} TRACKING_CLOCK_BITS;
typedef enum { KCLR=0, KBIN1=4, KBIN2=8, KBIN3=12} KT_CLOCK_BITS;
typedef enum { IIAG_H=1, SAG_H=2, SRG_H=4, R1_D3=8} CCD_CLOCK_BITS;
typedef enum { CLR_SELECT=1, R3_D1=2, MICRO_CLK_SELECT=4,
	PLD_TRIGGER=8 } READOUT_CONTROL_BITS;

typedef PAR_ERROR (*PDeviceIO)(DWORD, void *, DWORD, void *, DWORD );

extern void LPTInitPort(void);
extern void LPTCameraOut( OUTPUT_REGISTER reg, unsigned char val);
extern PAR_ERROR LPTSendMicroBlock(char *p, unsigned long *len);
extern PAR_ERROR LPTGetMicroBlock(char *p, unsigned long *len);
extern PAR_ERROR LPTGetPixels(CAMERA_TYPE cameraID, CCD_REQUEST ccd, USHORT *dest, 
					   short left, short len, short right, short horzBin,
					   short vertBin, short clearWidth);
extern PAR_ERROR LPTRVClockImagingCCD(CAMERA_TYPE cameraID, short onVertBin, short clearWidth);
extern PAR_ERROR LPTRVClockTrackingCCD(CAMERA_TYPE cameraID, short onVertBin, short clearWidth);
extern PAR_ERROR LPTRVClockST5CCCD(CAMERA_TYPE cameraID, short onVertBin, short clearWidth);
extern PAR_ERROR LPTDumpImagingLines(CAMERA_TYPE cameraID, short width, short len, short vertBin,
									 short iToVRatio);
extern PAR_ERROR LPTDumpTrackingLines(CAMERA_TYPE cameraID, short width, short len, short vertBin);
extern PAR_ERROR LPTDumpST5CLines(CAMERA_TYPE cameraID, short width, short len, short vertBin);
extern MY_LOGICAL LPTSetVdd(MY_LOGICAL raiseIt);
extern PAR_ERROR LPTClockAD(short len);
extern PAR_ERROR LPTClearImagingArray(CAMERA_TYPE cameraID, short height, short times);
extern PAR_ERROR LPTClearTrackingArray(CAMERA_TYPE cameraID, short height, short times);
extern PAR_ERROR LPTSetIRQL(SetIRQLParams *pParams);
extern PAR_ERROR LPTGetIRQL(GetIRQLResults *pResults);
extern PAR_ERROR LPTGetUSTimer(GetUSTimerResults *pResults);
extern PAR_ERROR WINDeviceIOControl(DWORD cmd, void *pParams, DWORD paramsSize,
							 void *pResults, DWORD resultsSize);
extern PAR_ERROR OpenCloseLPTLDevice(PAR_COMMAND Command, OpenDeviceParams *Params);
extern PAR_ERROR GetLPTLDriverInfo(GetDriverInfoParams *Params, PVOID *Results);

#endif
