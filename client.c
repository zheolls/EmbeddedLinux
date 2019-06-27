#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define PORT 9415  		// The port which is communicate with server
#define LENGTH 	256          		// Buffer length

int main(int argc, char *argv[])
{
	    int iR;					         //
		int nsockfd;               		// New Socket file descriptor
    	int sockfd;                        	// Socket file descriptor
    	int num;                    		// Counter of received bytes  
		int sin_size;
    	char revbuf[LENGTH];       		// Receive buffer
	    char sdbuf[LENGTH];                     // Send buffer
    	struct sockaddr_in remote_addr;    	// Host address information
    	/* Check parameters number */
    	if (argc != 2)                     
    	{    
        	printf ("Usage: client HOST IP (ex: ./client 192.168.7.239).\n");
        	return (0);
    	}

    	/* Get the Socket file descriptor */
    	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    	{
        	printf("ERROR: Failed to obtain Socket Descriptor!\n");
        	return (0);
    	}
    
    	/* Fill the socket address struct */
    	remote_addr.sin_family = AF_INET;              	// Protocol Family
    	remote_addr.sin_port = htons(PORT);           		// Port number
        remote_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
    	//inet_pton(AF_INET, argv[1], &remote_addr.sin_addr); 	// Net Address
    	//memset (remote_addr.sin_zero,0,8);                 	// Flush the rest of struct

    	/* Try to connect the remote */
    	if (nsockfd=connect(sockfd, (struct sockaddr *)&remote_addr,  sizeof(remote_addr)) < 0 ) 
    	{
        	printf ("ERROR: Failed to connect to the host!\n");
        	return (0);
    	} else {
        	printf ("OK: Have connected to the %s\n",argv[1]);
    	}

		
        
			/* Try to connect the server */
			printf("You can enter string to send, and press 'exit' to end the connect.\n");
			 
			// while(1)
			// {      
				
				/* 发送数据------------ */
			iR=(int)scanf("%s",sdbuf);
			if((num=send(sockfd,sdbuf,LENGTH,0))==-1)
			{
				printf("ERROR:Fail to send string\n");
				exit(1);
			}
			sin_size=sizeof(remote_addr);
			
			if (num<0)
				exit(1);
			printf("OK:Sent %d bytes sucessful,please enter again.\n",num);
			memset(revbuf, 0, LENGTH);
			revbuf[0] = '\0';
			num=recv(sockfd, revbuf, LENGTH, 0);
			if (num<0)
				exit(1);
			else
			{
				revbuf[num]='\0';
				printf("recvfrom server: %s\n",revbuf);
			}
		// }
    	
		printf("Exit connect,Byebye!\n");
    	close (sockfd);
    	return(0);
}
