
#include "tundev.h"
#include <stdio.h>
#include <unistd.h>
#include <thread>
#include "serial.h"
#include <string.h>
#include "serial_protol.h"

#include <list>
#include <mutex>
#include <memory>
using namespace std;


struct datanode {
	datanode(unsigned char * inbuff, int inlen)
	{
		len = inlen > 4096 ? 4096 : inlen;
		if (len)
			memcpy(buff, inbuff, len);
	}
	unsigned char buff[4096];
	int len;
};

mutex mtx_list;
list<shared_ptr< datanode>> dateList;

void ReadTunThread(int tunfd)
{
	fprintf(stderr, "tun thread\n");
	unsigned char inBuffer[4096];
	while (1) {
		int count = read(tunfd, inBuffer, sizeof(inBuffer));
		if (count > 0) {
			lock_guard<mutex> lck(mtx_list);
			dateList.emplace_back(make_shared<datanode>(inBuffer, count));
		} else {
			fprintf(stderr, "tun error = %d\n", count);
		}
	}
}



void MasterSerialThread(int serfd, int tunfd)
{
	fprintf(stderr, "Master serial thread\n");
	unsigned char inBuffer[4096];
	unsigned char outbuff[4096];
	shared_ptr<datanode> tmp;
	while (1) {
		if (!tmp) 
		{
			lock_guard<mutex> lck(mtx_list);
			if (!dateList.empty()) {
				tmp = std::move(*dateList.begin());
				dateList.erase(dateList.begin());
			}
		}
		//master send
		ser_data src;
		if (!tmp) {
			src.buff = nullptr;
			src.len = 0;
		} else {
			src.buff = tmp->buff;
			src.len = tmp->len;
		}
		ser_data dst;
		dst.buff = outbuff;
		dst.len = sizeof(outbuff);
		int len = serialEncode(&src, &dst, 0);
		int writelen = write(serfd, outbuff, len);
		//master read
		int readlen = read(serfd, inBuffer, sizeof(inBuffer));

		src.buff = inBuffer;
		src.len = readlen;

		dst.buff = outbuff;
		dst.len = sizeof(outbuff);

		int packcnt = serialFindDecode(&src, &dst, 1);

		if (packcnt) {
			tmp.reset();
			write(tunfd, dst.buff, dst.len);
		}
	}
}


void SlaveSerialThread(int serfd, int tunfd)
{
	fprintf(stderr, "Slave serial thread\n");
	unsigned char inBuffer[4096];
	unsigned char outbuff[4096];
	shared_ptr<datanode> tmp;
	while (1) {
		ser_data dst;
		ser_data src;
		//slave read
		int readlen = read(serfd, inBuffer, sizeof(inBuffer));

		if (readlen <= 0) {
			continue;
		}

		src.buff = inBuffer;
		src.len = readlen;

		dst.buff = outbuff;
		dst.len = sizeof(outbuff);

		int packcnt = serialFindDecode(&src, &dst, 1);

		if (packcnt) {
			tmp.reset();
			write(tunfd, dst.buff, dst.len);
		}

		if (!tmp) {
			lock_guard<mutex> lck(mtx_list);
			if (!dateList.empty()) {
				tmp = std::move(*dateList.begin());
				dateList.erase(dateList.begin());
			}
		}
		//slave send
		
		if (!tmp) {
			src.buff = nullptr;
			src.len = 0;
		} else {
			src.buff = tmp->buff;
			src.len = tmp->len;
		}
		
		dst.buff = outbuff;
		dst.len = sizeof(outbuff);
		int len = serialEncode(&src, &dst, 0);
		int writelen = write(serfd, outbuff, len);

	}
}


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

static int ChkifCMD(int argc, char *argv[], const char * cmd)
{
	for (int i = 0; i < argc; i++) {
		if (!strcmp(cmd, argv[i])) {
			return 1;
		}
	}
	return 0;
}

int main(int argc, char* argv[])
{
	char* serialName = ChkCmdVal(argc, argv, "-s");

	if (!serialName) {
		printf("no serial port exit\n");
		return -1;
	}

	int masterFlag = ChkifCMD(argc, argv, "-m");

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

	if (masterFlag) {
		thread thread_serial(MasterSerialThread, serial, tunfd);
		thread_serial.detach();
	} else {
		thread thread_serial(SlaveSerialThread, serial, tunfd);
		thread_serial.detach();
	}

	//thread thread_tun(tun2serial, tunfd, serial);
	//thread thread_serial(serial2tun, serial, tunfd);
	printf("11111\n");
	sleep(1000);
}


