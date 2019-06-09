
#include "tundev.h"
#include <stdio.h>
#include <unistd.h>
#include <thread>
#include "serial.h"
#include "libserialport.h"
#include <string.h>
using namespace std;


void tun2serial(int tunfd, sp_port * serialport)
{
	unsigned char inBuffer[2048];
	unsigned char outBuffer[4096];

	// Incoming byte count
	ssize_t count;

	// Encoded data size
	unsigned long encodedLength = 0;

	// Serial error messages
	enum sp_return serialResult;

	while (1) {
		count = read(tunfd, inBuffer, sizeof(inBuffer));
		if (count < 0) {
			fprintf(stderr, "Could not read from interface\n");
		}

		// Encode data
		encodedLength = serialEncode(inBuffer, count, outBuffer, sizeof(outBuffer));

		// Write to serial port
		serialResult = sp_nonblocking_write(serialport, outBuffer, encodedLength);
		if (serialResult < 0) {
			fprintf(stderr, "Could not send data to serial port: %d\n", serialResult);
		}
	}
}

void serial2tun(sp_port* serialport, int tunfd)
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
	
	auto  serial = init_serial(serialName, 115200);

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


