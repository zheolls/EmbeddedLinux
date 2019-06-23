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
	//�����ַ�ָ�� ָ�򴫵ݽ�����ch��ַ
	char* pch = ch;
	//ͨ��calloc������һ��length���ȵ��ַ����飬���ص����ַ�ָ�롣
	char* subch = (char*)calloc(sizeof(char), length + 1);
	int i;
	//ֻ����C99��forѭ���вſ�����������������д�����棬��߼����ԡ�  
	pch = pch + pos;
	//��pchָ��ָ��posλ�á�  
	for (i = 0; i < length; i++)
	{
		subch[i] = *(pch++);
		//ѭ��������ֵ���顣  
	}
	subch[length] = '\0';//�����ַ�����������  
	return subch;       //���ط�����ַ������ַ��  
}
char* itoa(int num, char* str, int radix)
{/*������*/
	char index[] = "0123456789ABCDEF";
	unsigned unum;/*�м����*/
	int i = 0, j, k;
	/*ȷ��unum��ֵ*/
	if (radix == 10 && num < 0)/*ʮ���Ƹ���*/
	{
		unum = (unsigned)-num;
		str[i++] = '-';
	}
	else unum = (unsigned)num;/*�������*/
	/*ת��*/
	do {
		str[i++] = index[unum % (unsigned)radix];
		unum /= radix;
	} while (unum);
	str[i] = '\0';
	/*����*/
	if (str[0] == '-')
		k = 1;/*ʮ���Ƹ���*/
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
				//��������
				memset(revbuf, 0, LENGTH);

				if ((num = recvfrom(nsockfd, revbuf, LENGTH, 0,&addr_remote, (socklen_t * __restrict) &sin_size)) > 0)
				{
					revbuf[num] = '\0';
					if (!strncmp(revbuf,"1000",4)) {
						int result = init_dcm();
						if (result == 0) {
							strcpy(sdbuf, "����Ѿ���");
						}
						else
						{
							strcpy(sdbuf, "�����ʧ��");
						}
					}
					else if (!strncmp(revbuf, "1001", 4)) {
						setpwm = atoi(substring(revbuf,4,4));

						int result= ioctl(dcm_fd, DCM_IOCTRL_SETPWM, (setpwm * factor));
						if (result == 0) {
							strcpy(sdbuf, "���ת���������");
						}
						else
						{
							strcpy(sdbuf, "���ת������ʧ��");
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
							strcpy(sdbuf, "����ѹر�");
						}
						else
						{
							strcpy(sdbuf, "����ر�ʧ��");
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