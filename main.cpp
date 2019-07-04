
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
	unsigned char inBuffer[2048];
	unsigned char outBuffer[4096];

	// Incoming byte count
	ssize_t count;

	// Encoded data size
	unsigned long encodedLength = 0;

	while (1) {
		count = read(tunfd, inBuffer, sizeof(inBuffer));
		if (count > 0) {
			fprintf(stderr, "Could not read from interface\n");

			ser_data src;
			src.len = count;
			src.buff = inBuffer;

			ser_data dst;
			dst.len = sizeof(outBuffer);
			dst.buff = outBuffer;

			encodedLength = serialEncode(&src, &dst);

			write(serfd, outBuffer, encodedLength);
		}
	}
}

void serial2tun(int serfd, int tunfd)
{





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
		exit(0);
	}
	
	int  serial = UARTX_Init(serialName, 115200, 0 ,8 , 1 , 0);

	if (!serial) {
		printf("serial port %s init error \n", serialName);
	} else {
		printf("serial port %s init success \n", serialName);
	}

	char tunname[128] = "test_tun";
	int tunfd = tun_alloc(tunname);
	if (tunfd < 0) {
		printf("tun alloc error = %d\n", tunfd);
	}

	printf("read dev name = %s\n", tunname);

	thread thread_tun(tun2serial, tunfd, serial);
	thread thread_serial(serial2tun, serial, tunfd);

	sleep(1000);
}


