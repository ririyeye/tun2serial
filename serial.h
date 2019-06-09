#ifndef __serial_h___
#define __serial_h___


#include "libserialport.h"
sp_port * init_serial(char* serialPortName, int serialBaudRate);
int serialEncode(unsigned char* srcbuff, unsigned int srclen, unsigned char* dstbuff, int dstlen);



#endif // !__serial_h___
