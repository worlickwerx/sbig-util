/*

	ulptdrv.h - Contains the typedefs and prototypes for the
				low-level LPT drivers for the Universal Driver.

*/
#ifndef _ULPTDRV_
#define _ULPTDRV_

#include "sbigudrv.h"

/*

	Low-Level Driver Commands

*/
#define SBIG_TYPE 40000
#ifndef FILE_ANY_ACCESS
 #define FILE_ANY_ACCESS 0
#endif
#ifndef METHOD_BUFFERED
 #define METHOD_BUFFERED 0
#endif

/*

	Control Codes

	These control codes constitute the inteface
    between the SBIG Universal Driver Library and the
	Low-Level Windows LPT and Ethernet drivers.

	Even Linux targets need these defines as they are
	passed to the Ethernet Server.

*/
#ifndef CTL_CODE
#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
    ((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method))
#endif

// 1 - 5
#define IOCTL_INIT_PORT CTL_CODE(	\
              SBIG_TYPE,			\
              2101,					\
              METHOD_BUFFERED,		\
              FILE_ANY_ACCESS)
#define IOCTL_CAMERA_OUT CTL_CODE(	\
              SBIG_TYPE,			\
              2102,					\
              METHOD_BUFFERED,		\
              FILE_ANY_ACCESS)
#define IOCTL_COM_MICRO CTL_CODE(	\
              SBIG_TYPE,			\
              2103,					\
              METHOD_BUFFERED,		\
              FILE_ANY_ACCESS)
#define IOCTL_SEND_MICRO CTL_CODE(	\
              SBIG_TYPE,			\
              2104,					\
              METHOD_BUFFERED,		\
              FILE_ANY_ACCESS)
#define IOCTL_GET_MICRO CTL_CODE(	\
              SBIG_TYPE,			\
              2105,					\
              METHOD_BUFFERED,		\
              FILE_ANY_ACCESS)

// 6 - 10
#define IOCTL_GET_PIXELS CTL_CODE(	\
              SBIG_TYPE,			\
              2106,					\
              METHOD_BUFFERED,		\
              FILE_ANY_ACCESS)
#define IOCTL_VCLOCK_ICCD CTL_CODE(	\
              SBIG_TYPE,			\
              2107,					\
              METHOD_BUFFERED,		\
              FILE_ANY_ACCESS)
#define IOCTL_VCLOCK_TCCD CTL_CODE(	\
              SBIG_TYPE,			\
              2108,					\
              METHOD_BUFFERED,		\
              FILE_ANY_ACCESS)
#define IOCTL_VCLOCK_5CCD CTL_CODE(	\
              SBIG_TYPE,			\
              2109,					\
              METHOD_BUFFERED,		\
              FILE_ANY_ACCESS)
#define IOCTL_DUMP_ILINES CTL_CODE(	\
              SBIG_TYPE,			\
              2110,					\
              METHOD_BUFFERED,		\
              FILE_ANY_ACCESS)

// 11 - 15
#define IOCTL_DUMP_TLINES CTL_CODE(	\
              SBIG_TYPE,			\
              2111,					\
              METHOD_BUFFERED,		\
              FILE_ANY_ACCESS)
#define IOCTL_DUMP_5LINES CTL_CODE(	\
              SBIG_TYPE,			\
              2112,					\
              METHOD_BUFFERED,		\
              FILE_ANY_ACCESS)
#define IOCTL_GET_US_TIMER CTL_CODE(\
              SBIG_TYPE,			\
              2113,					\
              METHOD_BUFFERED,		\
              FILE_ANY_ACCESS)
#define IOCTL_SET_VDD CTL_CODE(	\
              SBIG_TYPE,			\
              2114,					\
              METHOD_BUFFERED,		\
              FILE_ANY_ACCESS)
#define IOCTL_CLOCK_AD CTL_CODE(	\
              SBIG_TYPE,			\
              2115,					\
              METHOD_BUFFERED,		\
              FILE_ANY_ACCESS)

// 16 - 20
#define IOCTL_CLEAR_ICCD CTL_CODE(	\
              SBIG_TYPE,			\
              2116,					\
              METHOD_BUFFERED,		\
              FILE_ANY_ACCESS)
#define IOCTL_CLEAR_TCCD CTL_CODE(	\
              SBIG_TYPE,			\
              2117,					\
              METHOD_BUFFERED,		\
              FILE_ANY_ACCESS)
#define IOCTL_SET_IRQL CTL_CODE(	\
              SBIG_TYPE,			\
              2118,					\
              METHOD_BUFFERED,		\
              FILE_ANY_ACCESS)
#define IOCTL_GET_IRQL CTL_CODE(	\
              SBIG_TYPE,			\
              2119,					\
              METHOD_BUFFERED,		\
              FILE_ANY_ACCESS)
#define IOCTL_GET_DRIVER_INFO CTL_CODE(	\
              SBIG_TYPE,			\
              2120,					\
              METHOD_BUFFERED,		\
              FILE_ANY_ACCESS)

// 21 - 25
#define IOCTL_GET_AREA CTL_CODE(	\
              SBIG_TYPE,			\
              2121,					\
              METHOD_BUFFERED,		\
              FILE_ANY_ACCESS)
#define IOCTL_SET_PORT_ADDRESS CTL_CODE(\
              SBIG_TYPE,			\
              2122,					\
              METHOD_BUFFERED,		\
              FILE_ANY_ACCESS)
#define IOCTL_SHUT_DOWN_SERVER CTL_CODE(\
              SBIG_TYPE,			\
              2123,					\
              METHOD_BUFFERED,		\
              FILE_ANY_ACCESS)

/*

	Linux Driver Control Defines

	Note that these had to be renamed from IOCTL_XX to
    LIOCTL_XXX since the names conflicted with the predefined
	Windows IOCTLs and both need to exist under Linux to be
	compatible with the Ethernet server tool.

*/
#if TARGET == ENV_LINUX
 #define IOCTL_BASE   0xbb

 #define LIOCTL_GET_JIFFIES      _IOWR (IOCTL_BASE, 1,  char *)
 #define LIOCTL_GET_HZ           _IOWR (IOCTL_BASE, 2,  char *)
 #define LIOCTL_GET_DRIVER_INFO  _IOWR (IOCTL_BASE, 3,  char *)
 #define LIOCTL_SET_VDD          _IOWR (IOCTL_BASE, 4,  char *)
 #define LIOCTL_GET_LAST_ERROR   _IOWR (IOCTL_BASE, 5,  char *)

 #define LIOCTL_SEND_MICRO_BLOCK _IOWR (IOCTL_BASE, 10, char *)
 #define LIOCTL_GET_MICRO_BLOCK  _IOWR (IOCTL_BASE, 11, char *)
 #define LIOCTL_SUBMIT_IN_URB    _IOWR (IOCTL_BASE, 12, char *)
 #define LIOCTL_GET_IN_URB       _IOWR (IOCTL_BASE, 13, char *)

 #define LIOCTL_INIT_PORT        _IO   (IOCTL_BASE, 20)
 #define LIOCTL_CAMERA_OUT       _IOWR (IOCTL_BASE, 21, char *)
 #define LIOCTL_CLEAR_IMAG_CCD   _IOWR (IOCTL_BASE, 22, char *)
 #define LIOCTL_CLEAR_TRAC_CCD   _IOWR (IOCTL_BASE, 23, char *)
 #define LIOCTL_GET_PIXELS       _IOWR (IOCTL_BASE, 24, char *)
 #define LIOCTL_GET_AREA         _IOWR (IOCTL_BASE, 25, char *)
 #define LIOCTL_DUMP_ILINES      _IOWR (IOCTL_BASE, 26, char *)
 #define LIOCTL_DUMP_TLINES      _IOWR (IOCTL_BASE, 27, char *)
 #define LIOCTL_DUMP_5LINES      _IOWR (IOCTL_BASE, 28, char *)
 #define LIOCTL_CLOCK_AD         _IOWR (IOCTL_BASE, 29, char *)
 #define LIOCTL_REALLOCATE_PORTS _IOWR (IOCTL_BASE, 30, char *)
 #define LIOCTL_SET_BUFFER_SIZE  _IOW  (IOCTL_BASE, 31, unsigned short)
 #define LIOCTL_GET_BUFFER_SIZE  _IO   (IOCTL_BASE, 32)
 #define LIOCTL_TEST_COMMAND     _IO   (IOCTL_BASE, 33)

 #define LIOCTL_SUBMIT_PIXEL_IN_URB    _IOWR (IOCTL_BASE, 34, char *)
 #define LIOCTL_GET_PIXEL_IN_URB       _IOWR (IOCTL_BASE, 35, char *)
 #define LIOCTL_GET_PIXEL_BLOCK        _IOWR (IOCTL_BASE, 36, char *)
#endif /* #if TARGET == ENV_LINUX */


/*

	Individual Command Struct Defines

	Note: Don't use enums as types because
		  Windows C compile makes them 4 bytes.

*/
typedef struct {
	unsigned char reg;
	unsigned char value;
} IOC_CAMERA_OUT_PARAMS;
typedef struct {
	short /* CAMERA_TYPE */ cameraID;
	short /* CCD_REQUEST */ ccd;
	short left;
	short len;
	short right;
	short horzBin;
	short vertBin;
	short clearWidth;
	short st237A;
	short st253;
} IOC_GET_PIXELS_PARAMS;
typedef struct {
	short /* CAMERA_TYPE */ cameraID;
	short onVertBin;
	short clearWidth;
} IOC_VCLOCK_CCD_PARAMS;
typedef struct {
	short /* CAMERA_TYPE */ cameraID;
	short width;
	short len;
	short vertBin;
	short vToHRatio;
	short st253;
} IOC_DUMP_LINES_PARAMS;
typedef struct {
	short /* CAMERA_TYPE */ cameraID;
	short height;
	short times;
} IOC_CLEAR_CCD_PARAMS;
typedef struct {
	short /* CAMERA_TYPE */ cameraID;
	short /* CCD_REQUEST */ ccd;
	short left;
	short len;
	short right;
	short horzBin;
	short vertBin;
	short clearWidth;
	short st237A;
	short height;
} IOC_GET_AREA_PARAMS;
typedef struct {
	short baseAddress;
} IOC_SET_PORT_ADDRESS_PARAMS;

/*

	Linux Only Structs

*/
#if TARGET == ENV_LINUX
typedef struct{
	unsigned char  *pBuffer;
	unsigned long  length;
} LinuxMicroblock;
typedef struct{
	IOC_GET_PIXELS_PARAMS  gpp;
	unsigned short *dest;
	unsigned long  length;
} LinuxGetPixelsParams;
typedef struct{
	unsigned short  raiseIt;
	unsigned short  vddWasLow;
} IocSetVdd;
#endif /* #if TARGET == ENV_LINUX */

#endif
