
#include <string.h>
#include <stdint.h>
#include "serial_protol.h"
#define HEAD_NUM (0x08) 
#define TAIL_NUM (0x07)



int serialEncode(struct ser_data * src, struct ser_data * dst)
{
	dst->buff[0] = HEAD_NUM;
	dst->buff[1] = src->len & 0xff;
	dst->buff[2] = (src->len >> 8) & 0xff;
	memcpy(&dst->buff[3], src->buff, src->len);
	dst->buff[3 + src->len] = TAIL_NUM;

	return src->len + 3;
}

int serialFindDecode(struct ser_data * src, struct ser_data * dst)
{
	for (int i = 0; i < src->len - 5; i++) {
		//find head
		unsigned char * findHead = src->buff + i;
		if (*findHead == HEAD_NUM) {
			//test rec len
			unsigned int findlen = findHead[1] | (findHead[2] << 8);

			unsigned int testlen = findlen + 4;
			//check rec num
			if (findlen && (testlen + i <= src->len)) {
				//check tail num
				unsigned char taildata = findHead[testlen - 1];
				if (taildata == TAIL_NUM) {
					dst->len = findlen;
					dst->buff = findHead + 3;
					return findlen;
				}
			}
		}
	}
	return -1;
}




