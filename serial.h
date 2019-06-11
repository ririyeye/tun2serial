#ifndef __serial_h___
#define __serial_h___


int UARTX_Init(char* portName, int baudrate, int fctl, int databit, int stopbit, int parity);
int serialEncode(unsigned char* srcbuff, unsigned int srclen, unsigned char* dstbuff, int dstlen);



#endif // !__serial_h___
