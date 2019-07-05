#include <stdio.h>
#include "serial_protol.h"


void priHex(unsigned char * buff, int len)
{
	for (int i = 0; i < 20; i++) {
		printf("%02x,", buff[i]);
	}
	printf("\n");
}

int main()
{
	unsigned char p[] = { 8, 8 ,8 ,8,8 };
	ser_data src;
	src.buff = p;
	src.len = 5;

	ser_data dstdata;
	unsigned char dstbuff[250] = { 1,2,3,4,5,6,7,8,9};
	dstdata.buff = &dstbuff[8];
	dstdata.len = 200;
	serialEncode(&src, &dstdata);

	priHex(dstbuff, 20);

	unsigned char outbuff[250];
	ser_data outSer;
	ser_data deSrc;
	deSrc.buff = dstbuff;
	deSrc.len = 250;
	serialFindDecode(&deSrc, &outSer);



}


