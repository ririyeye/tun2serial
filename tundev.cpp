#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include "tundev.h"
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/if_tun.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <sys/ioctl.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <net/route.h>

int sethostaddr(const char* dev)
{
	struct ifreq ifr;
	bzero(&ifr, sizeof(ifr));
	strcpy(ifr.ifr_name, dev);
	struct sockaddr_in addr;
	bzero(&addr, sizeof addr);
	addr.sin_family = AF_INET;
	inet_pton(AF_INET, tip, &addr.sin_addr);
	//addr.sin_addr.s_addr = htonl(0xc0a80001);
	bcopy(&addr, &ifr.ifr_addr, sizeof addr);
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
		return sockfd;
	int err = 0;
	// ifconfig tun0 192.168.0.1
	if ((err = ioctl(sockfd, SIOCSIFADDR, (void *)&ifr)) < 0) {
		perror("ioctl SIOCSIFADDR");
		goto done;
	}
	// ifup tun0 其实就是启动tun0
	if ((err = ioctl(sockfd, SIOCGIFFLAGS, (void *)&ifr)) < 0) {
		perror("ioctl SIOCGIFFLAGS");
		goto done;
	}
	ifr.ifr_flags |= IFF_UP;
	if ((err = ioctl(sockfd, SIOCSIFFLAGS, (void *)&ifr)) < 0) {
		perror("ioctl SIOCSIFFLAGS");
		goto done;
	}
	// ifconfig tun0 192.168.0.1/24 # 配置子网掩码
	inet_pton(AF_INET, "255.255.255.0", &addr.sin_addr);
	bcopy(&addr, &ifr.ifr_netmask, sizeof addr);
	if ((err = ioctl(sockfd, SIOCSIFNETMASK, (void *)&ifr)) < 0) {
		perror("ioctl SIOCSIFNETMASK");
		goto done;
	}
	//gateway
#ifdef GateWay
	struct rtentry  rt;
	struct sockaddr_in sin; 

	memset(&rt, 0, sizeof(struct rtentry));
	memset(&sin, 0, sizeof(struct sockaddr_in));
	sin.sin_family = AF_INET;
	sin.sin_port = 0;
	if (inet_aton(GateWay, &sin.sin_addr) < 0) {
		perror("inet_aton error\n");
		goto done;
	}
	memcpy(&rt.rt_gateway, &sin, sizeof(struct sockaddr_in));
	((struct sockaddr_in *)&rt.rt_dst)->sin_family = AF_INET;
	((struct sockaddr_in *)&rt.rt_genmask)->sin_family = AF_INET;
	rt.rt_flags = RTF_GATEWAY;
	if (ioctl(sockfd, SIOCADDRT, &rt) < 0) {
		perror("ioctl(SIOCADDRT) error in set_default_route\n");
		goto done;
	}
#endif

done:
	close(sockfd);
	return err;
}

int tun_alloc(char dev[IFNAMSIZ])
{
	struct ifreq ifr;
	int fd, err;

	if ((fd = open("/dev/net/tun", O_RDWR)) < 0) {
		perror("open");
		return -1;
	}

	bzero(&ifr, sizeof(ifr));
	ifr.ifr_flags = IFF_TUN | IFF_NO_PI; // tun设备不包含以太网头部,而tap包含,仅此而已

	if (*dev) {
		strncpy(ifr.ifr_name, dev, IFNAMSIZ);
	}

	if ((err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0) {
		perror("ioctl TUNSETIFF");
		close(fd);
		return err;
	}
	strcpy(dev, ifr.ifr_name);
	if ((err = sethostaddr(dev)) < 0) // 设定地址等信息
		return err;

	return fd;
}
