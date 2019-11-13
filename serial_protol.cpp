
#include <string.h>
#include <stdint.h>
#include "serial_protol.h"
#include <stdio.h>


static uint32_t crc_make(unsigned char* ptr, int len, uint32_t firstcrc)
{
	uint32_t crc = firstcrc;
	char i;
	int ptrindex = 0;
	while (len != 0) {
		if (len < 0)
			len = 0;
		crc ^= ptr[ptrindex];
		for (i = 0; i < 8; i++) {
			if ((crc & 0x0001) == 0)
				crc = crc >> 1;
			else {
				crc = crc >> 1;
				crc ^= 0xa001;
			}
		}
		len -= 1;
		ptrindex++;
	}
	return crc;
}

static int crc_check(unsigned int len, unsigned char *Buff, unsigned int firstcrc, unsigned char *match_byte)
{
	unsigned int crc0, crc1;

	if (match_byte == NULL) {
		if (len > 2) {
			crc0 = crc_make(Buff, (len - 2), firstcrc);
			crc1 = (Buff[len - 1] << 8) + Buff[len - 2];
		} else
			return -1;
	} else {
		crc0 = crc_make(Buff, len, firstcrc);
		crc1 = (match_byte[1] << 8) + match_byte[0];
	}

	if (crc0 == crc1)
		return 0;
	else
		return -2;
}



int serialEncode(struct ser_data * src, struct ser_data * dst, int seq)
{
	dst->buff[0] = 0xaa;
	dst->buff[1] = 0xaa;
	dst->buff[2] = 1;
	dst->buff[3] = seq;

	int wrlen = 8 + src->len;

	dst->buff[4] = wrlen >> 8;
	dst->buff[5] = wrlen & 0xff;

	memcpy(&dst->buff[6], &src->buff[0], src->len);

	unsigned int mkcrc = crc_make(dst->buff, wrlen - 2, 0xffff);

	dst->buff[wrlen - 2] = mkcrc & 0xff;
	dst->buff[wrlen - 1] = mkcrc >> 8 & 0xff;

	return wrlen;
}

int serialFindDecode(struct ser_data * src, struct ser_data * dst, int dstsz)
{
	unsigned char * rxbuf = src->buff;
	int num = src->len;
	unsigned int checked_size = 0;

	for (int i = 0; i < num;) {
		int remainLen = num - i;
		if ((rxbuf[i] == 0XAA) && (rxbuf[i + 1] == 0XAA)) {
			if (remainLen > 20) {
				int recpackLen = rxbuf[i + 4] << 8 | rxbuf[i + 5];
				if (remainLen <= recpackLen) {
					if (crc_check(recpackLen, &(*(rxbuf + i)), 0XFFFF, NULL) == 0) {

						memcpy(dst[checked_size].buff, &rxbuf[i + 6], recpackLen - 8);
						dst[checked_size].len = recpackLen - 8;

						checked_size++;
						i += recpackLen;
						continue;
					}
				}
			}
		}
		i++;
	}
	return checked_size;
}




