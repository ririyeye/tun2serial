
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
#include <iostream>

//#include "communicate.h"

//宏定义  
#define FALSE  -1  
#define TRUE   0  

#define TIMEOUT_SEC(buflen,baud) (buflen*20/baud+2)  //接收超时   
#define TIMEOUT_USEC 0   

portinfo_t  portinfo;    //串口用结构体

						 //端口路径
const char *TTY_DEV[] = { "/dev/ttyAMA0",   //uart0
"/dev/ttyAMA1",   //uart1
"/dev/ttyAMA2",   //uart2
};

/*---------------------------内部调用-----------------------------*/
static int UARTx_Open(int fd, int com_port);
static void UARTx_Close(int fd);
static int UARTx_Set(int fdcom, portinfo_t uartinfo);


/*----------------------------------------------------------------*/

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
//                          UARTX相关                        
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/**
* 名称：     UARTx_Open
* 功能：     打开串口，并返回串口设备文件描述
* 入口参数： fd:文件描述符     com_port :串口号(uart0,uart1,uart2)
* 出口参数： 正确返回0；错误返回 -1
* 返回值：
*/

static int UARTx_Open(int fd, int com_port)
{

	fd = open(TTY_DEV[com_port], O_RDWR | O_NOCTTY | O_NONBLOCK);

	if (FALSE == fd)
	{
		perror("Can't Open Serial Port");
		return(FALSE);
	}

	//恢复串口为阻塞状态
	//非阻塞：fcntl(fd,F_SETFL,FNDELAY)
	//阻塞：fcntl(fd,F_SETFL,0)
	if (fcntl(fd, F_SETFL, 0) < 0)
	{
		printf("fcntl failed!\n");
		return(FALSE);
	}
	else
	{
		printf("fcntl=%d\n", fcntl(fd, F_SETFL, 0));
	}

	//测试是否为终端设备
	if (0 == isatty(fd))
	{
		printf("standard input is not a terminal device\n");
		return(FALSE);
	}
	else
	{
		printf("isatty success!\n");
	}

	printf("fd->open=%d\n", fd);
	return (fd);
}

/**
* 名称：     UARTx_Close
* 功能：     关闭串口
* 入口参数： fd:文件描述符
* 出口参数：
* 返回值：
*/
static void UARTx_Close(int fd)
{
	close(fd);
}

/**
* 名称：     UARTx_Set
* 功能：     设置串口相关参数
* 入口参数： fd:文件描述符 , uartinfo:串口配置结构体
* 出口参数： 成功：TURE  失败：FALSE
* 返回值：
*/
static int UARTx_Set(int fd, portinfo_t uartinfo)
{
	int i = 0;
	int baudrate = 0, fctl = 0, databit = 0, stopbit = 0, parity = 0;

	int   baudrate_arr[] = { B921600 , B576000 , B460800 , B230400 , B115200, B38400	,B19200, B9600, B4800, B2400, B1200, B300 };
	int   name_arr[] = { 921600 , 576000 , 460800 , 230400,115200, 38400, 19200,  9600,  4800,  2400,  1200,  300 };

	struct termios options;

	/*tcgetattr(fd,&options)得到与fd指向对象的相关参数，
	并将它们保存于options,该函数还可以测试配置是否正确，
	该串口是否可用等。若调用成功，函数返回值为0，若调用失败，函数返回值为1.
	*/
	if (tcgetattr(fd, &options) != 0)
	{
		perror("SetupSerial 1");
		return(FALSE);
	}

	//设置串口输入、输出波特率
	baudrate = uartinfo.baudrate;

	for (i = 0; i < sizeof(baudrate_arr) / sizeof(baudrate_arr[0]); i++)
	{
		if (baudrate == name_arr[i])
		{
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
	fctl = uartinfo.fctl;

	switch (fctl)
	{
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
	databit = uartinfo.databit;
	options.c_cflag &= ~CSIZE;
	switch (databit)
	{
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
		return (FALSE);
	}

	//设置停止位
	stopbit = uartinfo.stopbit;
	switch (stopbit)
	{
	case 1:
		options.c_cflag &= ~CSTOPB; break;
	case 2:
		options.c_cflag |= CSTOPB; break;
	default:
		fprintf(stderr, "Unsupported stop bits\n");
		return (FALSE);
	}

	//设置校验位
	parity = uartinfo.parity;
	switch (parity)
	{
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
		return (FALSE);
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
	if (tcsetattr(fd, TCSANOW, &options) != 0)
	{
		perror("com set error!\n");
		return (FALSE);
	}
	return (TRUE);
}

/*
* 名称：                UARTX_Init()
* 功能：                串口初始化
* 入口参数：        tty:       自定义设备号 0-UART0, 1-UART1, 2-UART2
*                   baudrate:  串口速度 300 1200 2400 4800 9600 19200 115200
*                   fctl:      数据流控制 0: none, 1: hardware, 2: software
*                   databits:  数据位   取值为5，6，7，8
*                   stopbits:  停止位   stop bits, 1, 2
*                   parity:    效验类型 parity 0: none, 1: odd, 2: even
*
* 出口参数：        错误返回 -1 ，正确返回设备描述符
**/
int UARTX_Init(int tty, int baudrate, int fctl, int databit, int stopbit, int parity)
{
	int fd = 0;

	portinfo.tty = tty;
	portinfo.baudrate = baudrate;    // baudrate: 19200
	portinfo.fctl = fctl;            // flow control: software
	portinfo.databit = databit;      // databit: 8   
	portinfo.stopbit = stopbit;      // stopbit: 1
	portinfo.parity = parity;        // parity: none

									 //尝试打开串口
	fd = UARTx_Open(fd, tty);

	if (fd < 0)  return FALSE;

	//设置串口相关参数
	if (UARTx_Set(fd, portinfo) == FALSE)
	{
		return FALSE;
	}
	else
	{
		return  fd;   //串口打开，配置成功，返回设备描述符
	}
}


