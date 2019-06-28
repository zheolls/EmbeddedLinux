#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#define PORT 9415
#define BACKLOG 10
#define LENGTH 512
#define DCM_IOCTRL_SETPWM (0x10)
#define DCM_TCNTB0 (16384)
int factor = DCM_TCNTB0 / 1024;
static int dcm_fd = -1;
char *DCM_DEV = "/dev/s3c2410-dc-motor0";
static int setpwm = 1003;
//1000 open dcm
//1001 set pwm
//1002 check pwm
//1003 close dcm
static int led_fd = -1;
static int led_fd2 = -1;
unsigned int LEDWORD = 0xff;
unsigned char led[8] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned char LEDCODE[10] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90};
void jmdelay(int n)
{
	int i, j, k;
	for (i = 0; i < n; i++)
		for (j = 0; j < 100; j++)
			for (k = 0; k < 100; k++)
				;
}

// char *recycle(char *ch, int length)
// {
// 	char *temp = (char *)calloc(sizeof(char), length + 1);
// 	for (int i = 0; i < length - 1; i++)
// 	{
// 		temp[i] = ch[i + 1];
// 	}
// 	temp[length - 1] = ch[0];
// 	temp[length] = '\0';
// 	return temp;
// }

void setdot(int ch, int state)
{
	ch-=1;
	int b = ch / 8;
	int a = ch % 8;
	if (state)
	{
		led[a] = led[a] | (0x01 << b);
	}
	else
	{
		led[a] = led[a] & (~(0x01 << b));
	}

	write(led_fd, led, 8);
}

char *substring(char *ch, int pos, int length)
{
	char *pch = ch;
	char *subch = (char *)calloc(sizeof(char), length + 1);
	int i;
	pch = pch + pos;
	for (i = 0; i < length; i++)
	{
		subch[i] = *(pch++);
	}
	subch[length] = '\0';
	return subch;
}
char *itoa(int num, char *str, int radix)
{
	char index[] = "0123456789ABCDEF";
	unsigned unum;
	int i = 0, j, k;
	if (radix == 10 && num < 0)
	{
		unum = (unsigned)-num;
		str[i++] = '-';
	}
	else
		unum = (unsigned)num;
	/*???*/
	do
	{
		str[i++] = index[unum % (unsigned)radix];
		unum /= radix;
	} while (unum);
	str[i] = '\0';
	if (str[0] == '-')
		k = 1;
	else
		k = 0;
	char temp;
	for (j = k; j <= (i - 1) / 2; j++)
	{
		temp = str[j];
		str[j] = str[i - 1 + k - j];
		str[i - 1 + k - j] = temp;
	}
	return str;
}

int init_dcm()
{

	if ((dcm_fd = open(DCM_DEV, O_WRONLY)) < 0)
	{
		printf("Error opening %s device\n", DCM_DEV);
		return 1;
	}
	else
	{
		setpwm = 0;
		ioctl(dcm_fd, DCM_IOCTRL_SETPWM, 0);
		return 0;
	}
}

int socket_listner()
{
	int iR;
	int sockfd;  // Socket file descriptor
	int nsockfd; // New Socket file descriptor
	int num;
	int sin_size; // to store struct size
	char pipbuffer[10];
	char sdbuf[LENGTH];  // Send buffer
	char revbuf[LENGTH]; // Receive buffer
	struct sockaddr_in addr_local;
	struct sockaddr_in addr_remote;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("ERROR: Failed to obtain Socket Despcritor.\n");
		return (0);
	}
	else
	{
		printf("OK: Obtain Socket Despcritor sucessfully.\n");
	}

	/* Fill the local socket address struct */
	addr_local.sin_family = AF_INET;				// Protocol Family
	addr_local.sin_port = htons(PORT);				// Port number
	addr_local.sin_addr.s_addr = htonl(INADDR_ANY); // AutoFill local address
	memset(addr_local.sin_zero, 0, 8);				// Flush the rest of struct
	led_fd = open("/dev/s3c2410_led0", O_RDWR);
	led_fd2 = open("/dev/led0", O_RDWR);
	ioctl(led_fd, 0x12, 0xffc0);

	/*  Blind a special Port */
	if (bind(sockfd, (struct sockaddr *)&addr_local, sizeof(struct sockaddr)) == -1)
	{
		printf("ERROR: Failed to bind Port %d.\n", PORT);
		return (0);
	}
	else
	{
		printf("OK: Bind the Port %d sucessfully.\n", PORT);
	}

	/*  Listen remote connect/calling */
	if (listen(sockfd, BACKLOG) == -1)
	{
		printf("ERROR: Failed to listen Port %d.\n", PORT);
		return (0);
	}
	else
	{
		printf("OK: Listening the Port %d sucessfully.\n", PORT);
	}
	while (1)
	{
		pid_t pid;
		int fd[2];
		int fd2[2];
		if (pipe(fd) < 0 || pipe(fd2) < 0)
		{
			break;
		}
		printf("listner to port: %d\n", PORT);
		sin_size = sizeof(struct sockaddr);
		while (1)
		{
			nsockfd = accept(sockfd, (struct sockaddr *)&addr_remote, &sin_size);
			memset(revbuf, '\0', LENGTH);
			if ((num = recv(nsockfd, revbuf, LENGTH, 0)) > 0)
			{
				revbuf[num] = '\0';
				if (!strncmp(revbuf, "1000", 4))
				{
					int result = init_dcm();
					if (result == 0)
					{
						strcpy(sdbuf, "1000");
					}
					else
					{
						strcpy(sdbuf, "2000");
					}
				}
				else if (!strncmp(revbuf, "1001", 4))
				{
					setpwm = atoi(substring(revbuf, 4, 4));
					printf("Set pwm: %d\n", setpwm);

					int result = ioctl(dcm_fd, DCM_IOCTRL_SETPWM, (setpwm * factor));
					if (result != 0)
					{
						strcpy(sdbuf, "1001");
					}
					else
					{
						strcpy(sdbuf, "2000");
					}

					if (setpwm < 0)
					{
						LEDWORD = 0xbf;
					}
					else
					{
						LEDWORD =0xff;
					}
					
					int a = setpwm / 52;
					if (a<0)
						a=-a;
					LEDWORD = (LEDWORD << 8) | LEDCODE[a];
					ioctl(led_fd, 0x12, LEDWORD);
				}
				else if (!strncmp(revbuf, "1002", 4))
				{
					itoa(setpwm, sdbuf, 10);
				}
				else if (!strncmp(revbuf, "1003", 4))
				{
					int result = close(dcm_fd);
					// int result = ioctl(dcm_fd, DCM_IOCTRL_SETPWM, 0);
					if (result == 0)
					{
						strcpy(sdbuf, "1003");
						setpwm = 1003;
					}
					else
					{
						strcpy(sdbuf, "2000");
					}
					ioctl(led_fd, 0x12, 0xffff);
				}
				else if (!strncmp(revbuf, "1005", 4))
				{
					int a = atoi(substring(revbuf, 7, 1));
					if (revbuf[6] == '0')
					{
						ioctl(led_fd2, a, 0);
					}
					else if (revbuf[6] == '1')
					{
						ioctl(led_fd2, a, 1);
					}
				}

				else if (!strncmp(revbuf, "1006", 4))
				{
					int a = atoi(substring(revbuf, 6, 2));
					if (a == 0)
					{
						for (int i = 0; i < 64; i++)
						{
							setdot(i, 0);
						}
					}
					if (revbuf[5] == '0')
					{
						setdot(a, 0);
					}
					else
					{
						setdot(a, 1);
					}
				}
				sdbuf[5] = '\0';
				printf("Return %s\n", sdbuf);
				send(nsockfd, sdbuf, LENGTH, 0);
			}
			close(nsockfd);
		}
	}
}

int main()
{
	socket_listner();
}
