#include "libserialport.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct sp_port* init_serial(char * serialPortName ,int serialBaudRate)
{
	struct sp_port* serialPort;
	sp_get_port_by_name(serialPortName, &serialPort);

	enum sp_return status = sp_open(serialPort, SP_MODE_READ_WRITE);

	sp_set_bits(serialPort, 8);
	sp_set_parity(serialPort, SP_PARITY_NONE);
	sp_set_stopbits(serialPort, 1);
	sp_set_baudrate(serialPort, serialBaudRate);
	sp_set_xon_xoff(serialPort, SP_XONXOFF_DISABLED);
	sp_set_flowcontrol(serialPort, SP_FLOWCONTROL_NONE);

	if (status < 0) {
		fprintf(stderr, "Could not open serial port: ");
		switch (status) {
		case SP_ERR_ARG:
			fprintf(stderr, "Invalid argument\n");
			break;
		case SP_ERR_FAIL:
			fprintf(stderr, "System error\n");
			break;
		case SP_ERR_MEM:
			fprintf(stderr, "Memory allocation error\n");
			break;
		case SP_ERR_SUPP:
			fprintf(stderr, "Operation not supported by device\n");
			break;
		default:
			fprintf(stderr, "Unknown error\n");
			break;
		}
		return nullptr;
	}
	return serialPort;
}

int serialEncode(unsigned char* srcbuff, unsigned int srclen, unsigned char* dstbuff, int MaxDstLen)
{
	dstbuff[0] = 0x02;
	dstbuff[1] = srclen & 0xff;
	dstbuff[2] = (srclen >> 8) & 0xff;
	memcpy(&dstbuff[3], srcbuff, srclen);
	dstbuff[3 + srclen] = 0x01;

	return srclen + 3;
}


