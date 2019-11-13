
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
	while (1) {
		int count = read(tunfd, inBuffer, sizeof(inBuffer));
		if (count > 0) {
			fprintf(stderr, "tun read num = %d\n", count);
			int writelen = write(serfd, inBuffer, count);
		} else {
			fprintf(stderr, "tun error = %d\n", count);
		}
	}
}

void serial2tun(int serfd, int tunfd)
{
	fprintf(stderr, "serial thread\n");
	unsigned char inBuffer[4096];
	while (1) {
		int count = read(serfd, inBuffer, sizeof(inBuffer));
		if (count > 0) {
			fprintf(stderr, "ser read num = %d\n", count);
			int writelen = write(tunfd, inBuffer, count);
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
		exit(0);
	}

	int  serial = UARTX_Init(serialName, 115200, 0, 8, 1, 0);

	if (serial < 0) {
		printf("serial port %s init error \n", serialName);
	} else {
		printf("serial port %s init success ,fd = %d\n", serialName, serial);
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


