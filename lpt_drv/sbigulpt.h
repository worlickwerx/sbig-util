/*

	sbigulpt.h - Contains the prototypes for the LPT code

*/
#ifndef _SBIGULPT_
#define _SBIGULPT_

#define DOS_VERSION_BCD			0x0400						/* DOS version in BCD */
#define DOS_VERSION_STRING  	"SBIGULPT Ver 4.0D" 		/* DOS version string */
#define VXD_VERSION_BCD			0x0407						/* VXD version in BCD */
#define VXD_VERSION_STRING  	"SBIGUDrv.VXD Ver 4.07"		/* VXD version string */
#define GENERIC_VERSION_BCD		0x0400						/* Generic version in BCD */
#define GENERIC_VERSION_STRING  "SBIGULPT.XXX Ver 4.0G"		/* Generic version string */

#if TARGET == ENV_WINSYS
typedef BOOLEAN (*PPARALLEL_TRY_ALLOCATE_ROUTINE)(IN PVOID TryAllocateContext);
typedef VOID    (*PPARALLEL_FREE_ROUTINE)(IN PVOID FreeContext);
typedef ULONG   (*PPARALLEL_QUERY_WAITERS_ROUTINE)(IN PVOID QueryAllocsContext);
#endif

typedef struct _DEVICE_EXTENSION {
#if TARGET == ENV_WINSYS
        // Points to the device object that contains this device extension.
        PDEVICE_OBJECT DeviceObject;
        // Points to the port device object that this class device is connected to.
        PDEVICE_OBJECT PortDeviceObject;
        // Work queue.  Manipulate with cancel spin lock.
        LIST_ENTRY WorkQueue;
        PIRP       CurrentIrp;
        // Holds the result of the get parallel port info request to the port driver.
        PHYSICAL_ADDRESS                PhysicalBaseAddress;
        ULONG                           Span;
        PPARALLEL_TRY_ALLOCATE_ROUTINE  TryAllocatePort;
        PPARALLEL_FREE_ROUTINE          FreePort;
        PPARALLEL_QUERY_WAITERS_ROUTINE QueryNumWaiters;
        PVOID                           FreePortContext;
        // Records whether we actually created the symbolic link name
        // at driver load time and the symbolic link itself.  If we didn't
        // create it, we won't try to destroy it when we unload.
        BOOLEAN        CreatedSymbolicLink;
        UNICODE_STRING SymbolicLinkName;

        //CCD local data are stored here:
        ULONG                       t1, t5, t60, t86400;
#endif
        //CCD local data are stored here:
        SetIRQLParams               NewIrql, OldIrql;
        PUCHAR                      BaseAddress;
        UCHAR                       control_out;
		ULONG						noBytesRd;
		ULONG						noBytesWr;
        UCHAR                       imaging_clocks_out;
        USHORT						state;
		USHORT						irqCount;
}DEVICE_EXTENSION, *PDEVICE_EXTENSION;

extern void ULPTInitPort(PDEVICE_EXTENSION pDevExt);
extern void ULPTCameraOut(PDEVICE_EXTENSION pDevExt, UCHAR reg, UCHAR val);
extern UCHAR ULPTCameraIn(PDEVICE_EXTENSION pDevExt, UCHAR reg);

extern PAR_ERROR ULPTGetUSTimer(PDEVICE_EXTENSION pDevExt,
                    GetUSTimerResults  *Results);
extern PAR_ERROR ULPTSendMicroBlock(PDEVICE_EXTENSION pDevExt,
                    UCHAR *pData, DWORD dataLen);
extern PAR_ERROR ULPTGetMicroBlock(PDEVICE_EXTENSION pDevExt,
					UCHAR *pData, DWORD dataLen);
extern PAR_ERROR ULPTGetPixels(PDEVICE_EXTENSION pDevExt,
						   IOC_GET_PIXELS_PARAMS *pParams,
						   unsigned short *dest, DWORD resultsSize);
extern PAR_ERROR ULPTGetArea(PDEVICE_EXTENSION pDevExt,
						   IOC_GET_AREA_PARAMS *pParams,
						   unsigned short *dest, DWORD resultsSize);
extern PAR_ERROR ULPTRVClockImagingCCD(PDEVICE_EXTENSION pDevExt,
							   IOC_VCLOCK_CCD_PARAMS *pParams);
extern PAR_ERROR ULPTRVClockTrackingCCD(PDEVICE_EXTENSION pDevExt,
							   IOC_VCLOCK_CCD_PARAMS *pParams);
extern PAR_ERROR ULPTRVClockST5CCCD(PDEVICE_EXTENSION pDevExt,
							   IOC_VCLOCK_CCD_PARAMS *pParams);
extern PAR_ERROR ULPTRVClockST253CCCD(PDEVICE_EXTENSION pDevExt,
							   IOC_VCLOCK_CCD_PARAMS *pParams);
extern PAR_ERROR ULPTDumpImagingLines(PDEVICE_EXTENSION pDevExt,
								  IOC_DUMP_LINES_PARAMS *pParams);
extern PAR_ERROR ULPTDumpTrackingLines(PDEVICE_EXTENSION pDevExt,
								  IOC_DUMP_LINES_PARAMS *pParams);
extern PAR_ERROR ULPTDumpST5CLines(PDEVICE_EXTENSION pDevExt,
								  IOC_DUMP_LINES_PARAMS *pParams);
extern PAR_ERROR ULPTDumpST253Lines(PDEVICE_EXTENSION pDevExt,
								  IOC_DUMP_LINES_PARAMS *pParams);
extern void ULPTSetVdd(PDEVICE_EXTENSION pDevExt, short raiseIt,
				   UCHAR *pVddWasLow);
extern PAR_ERROR ULPTClockAD(PDEVICE_EXTENSION pDevExt, short len);
extern PAR_ERROR ULPTClearImagingArray(PDEVICE_EXTENSION pDevExt,
								   IOC_CLEAR_CCD_PARAMS *pParams);
extern PAR_ERROR ULPTClearTrackingArray(PDEVICE_EXTENSION pDevExt,
								   IOC_CLEAR_CCD_PARAMS *pParams);
extern ULONG MyTickCount(void);
extern PAR_ERROR ULPTGetDriverInfo(GetDriverInfoParams *pParams,
							GetDriverInfoResults0 *pResults);

#endif
