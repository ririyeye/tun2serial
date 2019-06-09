
#include<stdio.h>      /*��׼�����������*/  
#include<stdlib.h>     /*��׼�����ⶨ��*/  
#include<unistd.h>     /*Unix ��׼��������*/  
#include<sys/types.h>   
#include <sys/time.h>
#include <time.h>
#include<sys/stat.h>     
#include<fcntl.h>      /*�ļ����ƶ���*/  
#include<termios.h>    /*PPSIX �ն˿��ƶ���*/  
#include<errno.h>      /*����Ŷ���*/  
#include<string.h> 
#include <iostream>

//#include "communicate.h"

//�궨��  
#define FALSE  -1  
#define TRUE   0  

#define TIMEOUT_SEC(buflen,baud) (buflen*20/baud+2)  //���ճ�ʱ   
#define TIMEOUT_USEC 0   

portinfo_t  portinfo;    //�����ýṹ��

						 //�˿�·��
const char *TTY_DEV[] = { "/dev/ttyAMA0",   //uart0
"/dev/ttyAMA1",   //uart1
"/dev/ttyAMA2",   //uart2
};

/*---------------------------�ڲ�����-----------------------------*/
static int UARTx_Open(int fd, int com_port);
static void UARTx_Close(int fd);
static int UARTx_Set(int fdcom, portinfo_t uartinfo);


/*----------------------------------------------------------------*/

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
//                          UARTX���                        
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/**
* ���ƣ�     UARTx_Open
* ���ܣ�     �򿪴��ڣ������ش����豸�ļ�����
* ��ڲ����� fd:�ļ�������     com_port :���ں�(uart0,uart1,uart2)
* ���ڲ����� ��ȷ����0�����󷵻� -1
* ����ֵ��
*/

static int UARTx_Open(int fd, int com_port)
{

	fd = open(TTY_DEV[com_port], O_RDWR | O_NOCTTY | O_NONBLOCK);

	if (FALSE == fd)
	{
		perror("Can't Open Serial Port");
		return(FALSE);
	}

	//�ָ�����Ϊ����״̬
	//��������fcntl(fd,F_SETFL,FNDELAY)
	//������fcntl(fd,F_SETFL,0)
	if (fcntl(fd, F_SETFL, 0) < 0)
	{
		printf("fcntl failed!\n");
		return(FALSE);
	}
	else
	{
		printf("fcntl=%d\n", fcntl(fd, F_SETFL, 0));
	}

	//�����Ƿ�Ϊ�ն��豸
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
* ���ƣ�     UARTx_Close
* ���ܣ�     �رմ���
* ��ڲ����� fd:�ļ�������
* ���ڲ�����
* ����ֵ��
*/
static void UARTx_Close(int fd)
{
	close(fd);
}

/**
* ���ƣ�     UARTx_Set
* ���ܣ�     ���ô�����ز���
* ��ڲ����� fd:�ļ������� , uartinfo:�������ýṹ��
* ���ڲ����� �ɹ���TURE  ʧ�ܣ�FALSE
* ����ֵ��
*/
static int UARTx_Set(int fd, portinfo_t uartinfo)
{
	int i = 0;
	int baudrate = 0, fctl = 0, databit = 0, stopbit = 0, parity = 0;

	int   baudrate_arr[] = { B921600 , B576000 , B460800 , B230400 , B115200, B38400	,B19200, B9600, B4800, B2400, B1200, B300 };
	int   name_arr[] = { 921600 , 576000 , 460800 , 230400,115200, 38400, 19200,  9600,  4800,  2400,  1200,  300 };

	struct termios options;

	/*tcgetattr(fd,&options)�õ���fdָ��������ز�����
	�������Ǳ�����options,�ú��������Բ��������Ƿ���ȷ��
	�ô����Ƿ���õȡ������óɹ�����������ֵΪ0��������ʧ�ܣ���������ֵΪ1.
	*/
	if (tcgetattr(fd, &options) != 0)
	{
		perror("SetupSerial 1");
		return(FALSE);
	}

	//���ô������롢���������
	baudrate = uartinfo.baudrate;

	for (i = 0; i < sizeof(baudrate_arr) / sizeof(baudrate_arr[0]); i++)
	{
		if (baudrate == name_arr[i])
		{
			printf("baudrate = %d\n", baudrate);
			cfsetispeed(&options, baudrate_arr[i]);   //����
			cfsetospeed(&options, baudrate_arr[i]);   //���
		}
	}

	//�޸Ŀ���ģʽ����֤���򲻻�ռ�ô���    
	options.c_cflag |= CLOCAL;
	//�޸Ŀ���ģʽ��ʹ���ܹ��Ӵ����ж�ȡ��������    
	options.c_cflag |= CREAD;

	//��������������
	fctl = uartinfo.fctl;

	switch (fctl)
	{
	case 0://��ʹ��������    
		   //			options.c_cflag &= ~CRTSCTS;
		break;
	case 1://ʹ��Ӳ��������    
		   //			options.c_cflag |= CRTSCTS;
		break;
	case 2://ʹ�����������    
		   //			options.c_cflag |= IXON | IXOFF | IXANY;
		break;
	}

	//��������λ������������־λ
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

	//����ֹͣλ
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

	//����У��λ
	parity = uartinfo.parity;
	switch (parity)
	{
	case 0: //����żУ��λ��    
		options.c_cflag &= ~PARENB;
		options.c_iflag &= ~INPCK;
		break;
	case 1://����Ϊ��У��        
		options.c_cflag |= (PARODD | PARENB);
		options.c_iflag |= INPCK;
		break;
	case 2://����ΪżУ��      
		options.c_cflag |= PARENB;
		options.c_cflag &= ~PARODD;
		options.c_iflag |= INPCK;
		break;
	default:
		fprintf(stderr, "Unsupported parity\n");
		return (FALSE);
	}

	//�޸����ģʽ��ԭʼ�������    
	options.c_oflag &= ~OPOST;

	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	//options.c_lflag &= ~(ISIG | ICANON);    

	//���õȴ�ʱ�����С�����ַ�    
	options.c_cc[VTIME] = 1; /* ��ȡһ���ַ��ȴ�1*(1/10)s */
	options.c_cc[VMIN] = 0; /* ��ȡ�ַ������ٸ���Ϊ1 */

	tcflush(fd, TCIFLUSH);//�����������������������ݣ����ǲ��ٶ�ȡ ˢ���յ������ݵ��ǲ���    
						  //options.c_iflag &= ~(BRKINT | IXON | IXOFF | IXANY | ICRNL | INLCR | IGNCR);
	options.c_iflag = 0;
	options.c_oflag &= ~(ONLCR | ONOCR | ONLRET);
	//�������� (���޸ĺ��termios�������õ������У�    
	if (tcsetattr(fd, TCSANOW, &options) != 0)
	{
		perror("com set error!\n");
		return (FALSE);
	}
	return (TRUE);
}

/*
* ���ƣ�                UARTX_Init()
* ���ܣ�                ���ڳ�ʼ��
* ��ڲ�����        tty:       �Զ����豸�� 0-UART0, 1-UART1, 2-UART2
*                   baudrate:  �����ٶ� 300 1200 2400 4800 9600 19200 115200
*                   fctl:      ���������� 0: none, 1: hardware, 2: software
*                   databits:  ����λ   ȡֵΪ5��6��7��8
*                   stopbits:  ֹͣλ   stop bits, 1, 2
*                   parity:    Ч������ parity 0: none, 1: odd, 2: even
*
* ���ڲ�����        ���󷵻� -1 ����ȷ�����豸������
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

									 //���Դ򿪴���
	fd = UARTx_Open(fd, tty);

	if (fd < 0)  return FALSE;

	//���ô�����ز���
	if (UARTx_Set(fd, portinfo) == FALSE)
	{
		return FALSE;
	}
	else
	{
		return  fd;   //���ڴ򿪣����óɹ��������豸������
	}
}


