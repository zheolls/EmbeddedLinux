#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#define PORT 8080
#define BACKLOG 10

#define LENGTH 512
#define DCM_IOCTRL_SETPWM 			(0x10)
#define DCM_TCNTB0				(16384)
int iR;
int factor = DCM_TCNTB0 / 1024;

int sockfd;
int nsockfd;
int num;
int sin_size;
char sdbuf[LENGTH], revbuf[LENGTH];
struct sockaddr_in addr_local;
struct sockaddr_in addr_remote;
static int dcm_fd = -1;
char* DCM_DEV = "/dev/s3c2410-dc-motor0";
int setpwm = 0;
//1000 open dcm 
//

char* substring(char* ch, int pos, int length)
{
	//定义字符指针 指向传递进来的ch地址
	char* pch = ch;
	//通过calloc来分配一个length长度的字符数组，返回的是字符指针。
	char* subch = (char*)calloc(sizeof(char), length + 1);
	int i;
	//只有在C99下for循环中才可以声明变量，这里写在外面，提高兼容性。  
	pch = pch + pos;
	//是pch指针指向pos位置。  
	for (i = 0; i < length; i++)
	{
		subch[i] = *(pch++);
		//循环遍历赋值数组。  
	}
	subch[length] = '\0';//加上字符串结束符。  
	return subch;       //返回分配的字符数组地址。  
}
char* itoa(int num, char* str, int radix)
{/*索引表*/
	char index[] = "0123456789ABCDEF";
	unsigned unum;/*中间变量*/
	int i = 0, j, k;
	/*确定unum的值*/
	if (radix == 10 && num < 0)/*十进制负数*/
	{
		unum = (unsigned)-num;
		str[i++] = '-';
	}
	else unum = (unsigned)num;/*其他情况*/
	/*转换*/
	do {
		str[i++] = index[unum % (unsigned)radix];
		unum /= radix;
	} while (unum);
	str[i] = '\0';
	/*逆序*/
	if (str[0] == '-')
		k = 1;/*十进制负数*/
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
int init_socket() {
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("ERROR: Failed to obtain Socket Despcritor.\n");
		return (0);
	}
	else {
		printf("OK: Obtain Socket Despcritor sucessfully.\n");
	}
	addr_local.sin_family = AF_INET;
	addr_local.sin_port = PORT;
	addr_local.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(addr_local.sin_zero, 0, 8);
	/*  Blind a special Port */
	if (bind(sockfd, (struct sockaddr*) & addr_local, sizeof(struct sockaddr)) == -1)
	{
		printf("ERROR: Failed to bind Port %d.\n", PORT);
		return (0);
	}
	else {
		printf("OK: Bind the Port %d sucessfully.\n", PORT);
	}

	/*  Listen remote connect/calling */
	if (listen(sockfd, BACKLOG) == -1)
	{
		printf("ERROR: Failed to listen Port %d.\n", PORT);
		return (0);
	}
	else {
		printf("OK: Listening the Port %d sucessfully.\n", PORT);
	}
	return 1;
}

int  init_dcm() {
	if ((dcm_fd = open(DCM_DEV, O_WRONLY)) < 0) {
		printf("Error opening %s device\n", DCM_DEV);
		return 1;
	}
	else
	{
		return 0;
	}
}


void socket_listner() {
	while (1)
	{
		sin_size = sizeof(struct sockaddr_in);

		/*  Wait a connection, and obtain a new socket file despriptor for single connection */
		if ((nsockfd = accept(sockfd, (struct sockaddr*) & addr_remote, (socklen_t * __restrict) & sin_size)) == -1)
		{
			printf("ERROR: Obtain new Socket Despcritor error.\n");
			continue;
		}
		else {
			printf("OK: Server has got connect from %s.\n", inet_ntoa(addr_remote.sin_addr));
		}


		/* Child process */
		if (!fork())
		{

			while (strcmp(revbuf, "exit") != 0)
			{
				//接收数据
				memset(revbuf, 0, LENGTH);

				if ((num = recvfrom(nsockfd, revbuf, LENGTH, 0,&addr_remote, (socklen_t * __restrict) &sin_size)) > 0)
				{
					revbuf[num] = '\0';
					if (!strncmp(revbuf,"1000",4)) {
						int result = init_dcm();
						if (result == 0) {
							strcpy(sdbuf, "电机已经打开");
						}
						else
						{
							strcpy(sdbuf, "电机打开失败");
						}
					}
					else if (!strncmp(revbuf, "1001", 4)) {
						setpwm = atoi(substring(revbuf,4,4));

						int result= ioctl(dcm_fd, DCM_IOCTRL_SETPWM, (setpwm * factor));
						if (result == 0) {
							strcpy(sdbuf, "电机转速设置完成");
						}
						else
						{
							strcpy(sdbuf, "电机转速设置失败");
						}
					}
					else if (!strncmp(revbuf, "1002", 4))
					{
						itoa(setpwm, sdbuf, 10);
	
					}
					else if (!strncmp(revbuf, "1003", 4))
					{
						int result = ioctl(dcm_fd, DCM_IOCTRL_SETPWM, 0);
						if (result == 0) {
							strcpy(sdbuf, "电机已关闭");
						}
						else
						{
							strcpy(sdbuf, "电机关闭失败");
						}
					}
					sendto(sockfd, sdbuf, LENGTH, 0, &addr_remote, (socklen_t)sin_size);
				}


			}
		}

		close(nsockfd);
		//while (waitpid(-1, NULL, WNOHANG) > 0);

	}
}
int main() {
	init_socket();
	init_dcm();
	socket_listner();
	

}