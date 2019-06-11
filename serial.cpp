#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<stdio.h>      /*标准输入输出定义*/  
#include<stdlib.h>     /*标准函数库定义*/  
#include<unistd.h>     /*Unix 标准函数定义*/  
#include<sys/types.h>   
#include <sys/time.h>
#include <time.h>
#include<sys/stat.h>     
#include<fcntl.h>      /*文件控制定义*/  
#include<termios.h>    /*PPSIX 终端控制定义*/  
#include<errno.h>      /*错误号定义*/  
#include<string.h>

static int UARTx_Set(int fd, int baudrate, int fctl, int databit, int stopbit, int parity)
{
	int i = 0;

	int   baudrate_arr[] = { B921600 , B576000 , B460800 , B230400 , B115200, B38400	,B19200, B9600, B4800, B2400, B1200, B300 };
	int   name_arr[] = { 921600 , 576000 , 460800 , 230400,115200, 38400, 19200,  9600,  4800,  2400,  1200,  300 };

	struct termios options;

	/*tcgetattr(fd,&options)得到与fd指向对象的相关参数，
	并将它们保存于options,该函数还可以测试配置是否正确，
	该串口是否可用等。若调用成功，函数返回值为0，若调用失败，函数返回值为1.
	*/
	if (tcgetattr(fd, &options) != 0) {
		perror("SetupSerial 1");
		return -1;
	}

	//设置串口输入、输出波特率
	baudrate = baudrate;

	for (i = 0; i < sizeof(baudrate_arr) / sizeof(baudrate_arr[0]); i++) {
		if (baudrate == name_arr[i]) {
			printf("baudrate = %d\n", baudrate);
			cfsetispeed(&options, baudrate_arr[i]);   //输入
			cfsetospeed(&options, baudrate_arr[i]);   //输出
		}
	}

	//修改控制模式，保证程序不会占用串口    
	options.c_cflag |= CLOCAL;
	//修改控制模式，使得能够从串口中读取输入数据    
	options.c_cflag |= CREAD;

	//设置数据流控制

	switch (fctl) {
	case 0://不使用流控制    
		   //			options.c_cflag &= ~CRTSCTS;
		break;
	case 1://使用硬件流控制    
		   //			options.c_cflag |= CRTSCTS;
		break;
	case 2://使用软件流控制    
		   //			options.c_cflag |= IXON | IXOFF | IXANY;
		break;
	}

	//设置数据位，屏蔽其它标志位
	options.c_cflag &= ~CSIZE;
	switch (databit) {
	case 5:
		options.c_cflag |= CS5;
		break;
	case 6:
		options.c_cflag |= CS6;
		break;
	case 7:
		options.c_cflag |= CS7;
		break;
	case 8:
		options.c_cflag |= CS8;
		break;
	default:
		fprintf(stderr, "Unsupported data size\n");
		return -1;
	}

	//设置停止位
	switch (stopbit) {
	case 1:
		options.c_cflag &= ~CSTOPB; break;
	case 2:
		options.c_cflag |= CSTOPB; break;
	default:
		fprintf(stderr, "Unsupported stop bits\n");
		return -1;
	}

	//设置校验位
	switch (parity) {
	case 0: //无奇偶校验位。    
		options.c_cflag &= ~PARENB;
		options.c_iflag &= ~INPCK;
		break;
	case 1://设置为奇校验        
		options.c_cflag |= (PARODD | PARENB);
		options.c_iflag |= INPCK;
		break;
	case 2://设置为偶校验      
		options.c_cflag |= PARENB;
		options.c_cflag &= ~PARODD;
		options.c_iflag |= INPCK;
		break;
	default:
		fprintf(stderr, "Unsupported parity\n");
		return -1;
	}

	//修改输出模式，原始数据输出    
	options.c_oflag &= ~OPOST;

	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	//options.c_lflag &= ~(ISIG | ICANON);    

	//设置等待时间和最小接收字符    
	options.c_cc[VTIME] = 1; /* 读取一个字符等待1*(1/10)s */
	options.c_cc[VMIN] = 0; /* 读取字符的最少个数为1 */

	tcflush(fd, TCIFLUSH);//如果发生数据溢出，接收数据，但是不再读取 刷新收到的数据但是不读    
						  //options.c_iflag &= ~(BRKINT | IXON | IXOFF | IXANY | ICRNL | INLCR | IGNCR);
	options.c_iflag = 0;
	options.c_oflag &= ~(ONLCR | ONOCR | ONLRET);
	//激活配置 (将修改后的termios数据设置到串口中）    
	if (tcsetattr(fd, TCSANOW, &options) != 0) {
		perror("com set error!\n");
		return  -1;
	}
	return fd;
}



static int UARTx_Open(const char * PortName, int com_port)
{
	int fd = open(PortName, O_RDWR | O_NOCTTY | O_NONBLOCK);

	if (0 > fd) {
		perror("Can't Open Serial Port");
		return -1;
	}

	//恢复串口为阻塞状态
	//非阻塞：fcntl(fd,F_SETFL,FNDELAY)
	//阻塞：fcntl(fd,F_SETFL,0)
	if (fcntl(fd, F_SETFL, 0) < 0) {
		printf("fcntl failed!\n");
		return -1;
	} else {
		printf("fcntl=%d\n", fcntl(fd, F_SETFL, 0));
	}

	//测试是否为终端设备
	if (0 == isatty(fd)) {
		printf("standard input is not a terminal device\n");
		return -1;
	} else {
		printf("isatty success!\n");
	}

	printf("fd->open=%d\n", fd);
	return (fd);
}

int UARTX_Init(char* portName, int baudrate, int fctl, int databit, int stopbit, int parity)
{
	int fd = 0;

	//尝试打开串口
	fd = UARTx_Open(portName, baudrate);

	if (fd < 0)  return fd;

	//设置串口相关参数
	if (UARTx_Set(fd, baudrate, fctl, databit, stopbit, parity) < 0) {
		close(fd);
		return -1;
	} else {
		return  fd;   //串口打开，配置成功，返回设备描述符
	}
}



int serialEncode(unsigned char* srcbuff, unsigned int srclen, unsigned char* dstbuff, int MaxDstLen)
{
	dstbuff[0] = 0x02;
	dstbuff[1] = srclen & 0xff;
	dstbuff[2] = (srclen >> 8) & 0xff;
	memcpy(&dstbuff[3], srcbuff, srclen);
	dstbuff[3 + srclen] = 0x01;

	return srclen + 3;
}


