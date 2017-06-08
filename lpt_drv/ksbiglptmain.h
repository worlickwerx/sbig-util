#ifndef _KSBIGLPTMAIN_H_
#define _KSBIGLPTMAIN_H_

// The buffer may be resized via IOCTL_SET_BUFFER_SIZE call
#define LDEFAULT_BUFFER_SIZE            4096

//========================================================================
struct private_data {
	unsigned long   flags;
	unsigned long   value;
	unsigned char   control_out;
	unsigned char   imaging_clocks_out;
	unsigned char   noBytesRd;
	unsigned char   noBytesWr;
	unsigned short  state;
	unsigned short  buffer_size;
	char           *buffer;
	void 		(*outb)(uint8_t data, unsigned int minor);
	uint8_t		(*inb)(unsigned int minor);
	unsigned int	minor;
};
#endif
