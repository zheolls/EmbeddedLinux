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
#define DCM_IOCTRL_SETPWM 			(0x10)
#define DCM_TCNTB0				(16384)
int factor = DCM_TCNTB0 / 1024;
static int dcm_fd = -1;
char* DCM_DEV = "/dev/s3c2410-dc-motor0";
int setpwm = 0;
//1000 open dcm 
//1001 set pwm
//1002 check pwm
//1003 close dcm

char* substring(char* ch, int pos, int length)
{
	char* pch = ch;
	char* subch = (char*)calloc(sizeof(char), length + 1);
	int i;
	pch = pch + pos;
	for (i = 0; i < length; i++)
	{
		subch[i] = *(pch++);
	}
	subch[length] = '\0';  
	return subch;     
}
char* itoa(int num, char* str, int radix)
{
	char index[] = "0123456789ABCDEF";
	unsigned unum;
	int i = 0, j, k;
	if (radix == 10 && num < 0)
	{
		unum = (unsigned)-num;
		str[i++] = '-';
	}
	else unum = (unsigned)num;
	/*???*/
	do {
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

int socket_listner() {
	int iR;
	int sockfd;                        // Socket file descriptor
	int nsockfd;               		// New Socket file descriptor
	int num;
	int sin_size;                      	// to store struct size
	char sdbuf[LENGTH];          	// Send buffer
	char revbuf[LENGTH];       		// Receive buffer
	struct sockaddr_in addr_local; 
	struct sockaddr_in addr_remote; 
//	char sendstr[16]= {"123456789 abcde"}; 
			
	/* Get the Socket file descriptor */  
	if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 )  
	{   
		printf ("ERROR: Failed to obtain Socket Despcritor.\n");
		return (0);
	} else {
		printf ("OK: Obtain Socket Despcritor sucessfully.\n");
	}

	/* Fill the local socket address struct */
	addr_local.sin_family = AF_INET;           	// Protocol Family
	addr_local.sin_port = htons (PORT);         	// Port number
	addr_local.sin_addr.s_addr  = htonl (INADDR_ANY);  // AutoFill local address
	memset (addr_local.sin_zero,0,8);          		// Flush the rest of struct

	/*  Blind a special Port */
	if( bind(sockfd, (struct sockaddr*)&addr_local, sizeof(struct sockaddr)) == -1 )
	{  
			printf ("ERROR: Failed to bind Port %d.\n",PORT);
		return (0);
	} else {
		printf("OK: Bind the Port %d sucessfully.\n",PORT);
	}

	/*  Listen remote connect/calling */
	if(listen(sockfd,BACKLOG) == -1)    
	{  
		printf ("ERROR: Failed to listen Port %d.\n", PORT);
		return (0);
	} else {
		printf ("OK: Listening the Port %d sucessfully.\n", PORT);
	}

	while (1)
	{
		printf("listner to port: %d\n", PORT);
		sin_size = sizeof(struct sockaddr);

		/*  Wait a connection, and obtain a new socket file despriptor for single connection */
		if ((nsockfd = accept(sockfd, (struct sockaddr *)&addr_remote, &sin_size)) < 0)
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
				//????????
				memset(revbuf, 0, LENGTH);

				if ((num = recvfrom(nsockfd, revbuf, LENGTH, 0,(struct sockaddr *)&addr_remote, (socklen_t *) &sin_size)) > 0)
				{
					revbuf[num] = '\0';
					if (!strncmp(revbuf,"1000",4)) {
						int result = init_dcm();
						if (result == 0) {
							strcpy(sdbuf, "1000");
						}
						else
						{
							strcpy(sdbuf, "2000");
						}
					}
					else if (!strncmp(revbuf, "1001", 4)) {
						setpwm = atoi(substring(revbuf,4,4));

						int result= ioctl(dcm_fd, DCM_IOCTRL_SETPWM, (setpwm * factor));
						if (result == 0) {
							strcpy(sdbuf, "1001");
						}
						else
						{
							strcpy(sdbuf, "2000");
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
							strcpy(sdbuf, "1003");
						}
						else
						{
							strcpy(sdbuf, "2000");
						}
					}
					sendto(sockfd, sdbuf, LENGTH, 0, (struct sockaddr *)&addr_remote, (socklen_t)sin_size);
				}


			}
		}
		close(nsockfd);
		while (waitpid(-1, NULL, WNOHANG) > 0);

	}
}

int main() {
	socket_listner();
}