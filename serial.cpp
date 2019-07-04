#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

static int UARTx_Set(int fd, int baudrate, int fctl, int databit, int stopbit, int parity)
{
	int i = 0;

	int   baudrate_arr[] = { B921600 , B576000 , B460800 , B230400 , B115200, B38400	,B19200, B9600, B4800, B2400, B1200, B300 };
	int   name_arr[] = { 921600 , 576000 , 460800 , 230400,115200, 38400, 19200,  9600,  4800,  2400,  1200,  300 };

	struct termios options;

	/*tcgetattr(fd,&options)�õ���fdָ��������ز�����
	�������Ǳ�����options,�ú��������Բ��������Ƿ���ȷ��
	�ô����Ƿ���õȡ������óɹ�����������ֵΪ0��������ʧ�ܣ���������ֵΪ1.
	*/
	if (tcgetattr(fd, &options) != 0) {
		perror("SetupSerial 1");
		return -1;
	}

	//���ô������롢���������
	baudrate = baudrate;

	for (i = 0; i < sizeof(baudrate_arr) / sizeof(baudrate_arr[0]); i++) {
		if (baudrate == name_arr[i]) {
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

	switch (fctl) {
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

	//����ֹͣλ
	switch (stopbit) {
	case 1:
		options.c_cflag &= ~CSTOPB; break;
	case 2:
		options.c_cflag |= CSTOPB; break;
	default:
		fprintf(stderr, "Unsupported stop bits\n");
		return -1;
	}

	//����У��λ
	switch (parity) {
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
		return -1;
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

	//�ָ�����Ϊ����״̬
	//��������fcntl(fd,F_SETFL,FNDELAY)
	//������fcntl(fd,F_SETFL,0)
	if (fcntl(fd, F_SETFL, 0) < 0) {
		printf("fcntl failed!\n");
		return -1;
	} else {
		printf("fcntl=%d\n", fcntl(fd, F_SETFL, 0));
	}

	//�����Ƿ�Ϊ�ն��豸
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

	//���Դ򿪴���
	fd = UARTx_Open(portName, baudrate);

	if (fd < 0)  return fd;

	//���ô�����ز���
	if (UARTx_Set(fd, baudrate, fctl, databit, stopbit, parity) < 0) {
		close(fd);
		return -1;
	} else {
		return  fd;   //���ڴ򿪣����óɹ��������豸������
	}
}



