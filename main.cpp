
#include "tundev.h"
#include <stdio.h>
#include <unistd.h>
#include <thread>
#include "serial.h"
#include <string.h>
#include "serial_protol.h"
using namespace std;


void tun2serial(int tunfd, int serfd)
{
	fprintf(stderr, "tun thread\n");
	unsigned char inBuffer[4096];
	unsigned char outBuffer[4096];
	while (1) {
		int count = read(tunfd, inBuffer, sizeof(inBuffer));
		if (count > 0) {

			ser_data src;
			src.buff = inBuffer;
			src.len = count;

			ser_data dst;
			dst.buff = outBuffer;
			dst.len = sizeof(outBuffer);

			int len = serialEncode(&src, &dst, 0);

			fprintf(stderr, "tun read num = %d , encode =%d\n", count, len);
			int writelen = write(serfd, outBuffer, len);
		} else {
			fprintf(stderr, "tun error = %d\n", count);
		}
	}
}




void serial2tun(int serfd, int tunfd)
{
	fprintf(stderr, "serial thread\n");
	unsigned char inBuffer[4096];
	unsigned char outbuff[10][4096];

	while (1) {
		int count = read(serfd, inBuffer, sizeof(inBuffer));
		if (count > 0) {

			ser_data src;
			ser_data dst[10];

			for (int i = 0; i < 10; i++) {
				dst->buff = &outbuff[i][0];
				dst->len = 4096;
			}

			src.buff = inBuffer;
			src.len = count;

			int packcnt = serialFindDecode(&src, dst, 10);

			fprintf(stderr, "ser read num = %d , pack cnt = %d\n", count, packcnt);

			if (packcnt == 0) {
				fprintf(stderr, "rec error pack\n");
			}

			for (int i = 0; i < packcnt; i++) {
				fprintf(stderr, "pack len = %d\n", dst[i].len);
				int writelen = write(tunfd, dst[i].buff, dst[i].len);
			}

		} else {
			if (count != 0) {
				fprintf(stderr, "ser error = %d\n", count);
			}
		}
	}
}


static char* ChkCmdVal(int argc, char* argv[], const char* cmd)
{
	for (int i = 0; i < argc; i++) {
		if (!strcmp(cmd, argv[i])) {
			if (i + 1 < argc) {
				return argv[i + 1];
			}
		}
	}
	return nullptr;
}



int main(int argc, char* argv[])
{
	char* serialName = ChkCmdVal(argc, argv, "-s");

	if (!serialName) {
		printf("no serial port exit\n");
		return -1;
	}

	int  serial = UARTX_Init(serialName, 115200, 0, 8, 1, 0);

	if (serial < 0) {
		printf("serial port %s init error \n", serialName);
		return -2;
	} else {
		printf("serial port %s init success ,fd = %d\n", serialName, serial);
	}

	char tunname[128] = "test_tun";
	int tunfd = tun_alloc(tunname);
	if (tunfd < 0) {
		printf("tun alloc error = %d\n", tunfd);
		return -3;
	}

	printf("read dev name = %s\n", tunname);

	thread thread_tun(tun2serial, tunfd, serial);
	thread thread_serial(serial2tun, serial, tunfd);

	sleep(1000);
}


