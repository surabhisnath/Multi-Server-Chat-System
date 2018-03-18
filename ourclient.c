// #include<stdio.h>
// #include<string.h>
// #include<stdlib.h>
// #include<netinet/in.h>
// #include<pthread.h>
// #include<sys/types.h>
// #include<sys/socket.h>

#include <stdio.h>//
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#define PORT 5000

void connect_with_server(int *socket_fd, struct sockaddr_in *serveraddr);

void sendandreceive(int var, int socket_fd);

int main()
{

	int socket_fd; 
	struct sockaddr_in serveraddr;
	connect_with_server(&socket_fd, &serveraddr);


	fd_set one;
	fd_set two;
	FD_ZERO(&one);
	FD_ZERO(&two);
	FD_SET(0, &one);
	FD_SET(socket_fd, &one);

	int max = socket_fd;

	while(1)
	{
		two = one;
		if(select(max+1, &two, NULL, NULL, NULL) == -1)
		{
			perror("Error in select");
			exit(4);
		}

		for(int i=0; i<max; i++)
		{
			if(FD_ISSET(i, &two))
				sendandreceive(i, socket_fd);
		}

	}

	printf("Client quited\n");
	close(socket_fd);
	return 0;
}


void connect_with_server(int *socket_fd, struct sockaddr_in *serveraddr)
{
	// if((*socket_fd = socket(AF_INET, SOCK_STREAM,0))<0)
	// {
	// 	printf("Error socket went WRONG");
	// 	return -1;
	// }

	// serveraddr->sin_family = AF_INET;
	// serveraddr->sin_port = htons(PORT);
	// server_addr->sin_addr.s_addr = inet_addr("127.0.0.1");
	// memset(&serveraddr, '0', sizeof(serveraddr));
	// if(inet_pton(AF_INET, "127.0.0.1", &serveraddr->sin_addr)<=0)
	// {
	// 	printf("Invalid address not supported\n");
	// 	return -1;
	// }

	// if(connect(*socket_fd,(struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
	// {
	// 	perror("Could not connect");
	// 	exit(1);
	// }


	if ((*socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Socket");
		exit(1);
	}
	serveraddr->sin_family = AF_INET;
	serveraddr->sin_port = htons(5000);
	serveraddr->sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(serveraddr->sin_zero, '\0', sizeof serveraddr->sin_zero);
	
	if(connect(*socket_fd, (struct sockaddr *)serveraddr, sizeof(struct sockaddr)) == -1) {
		perror("connect");
		exit(1);
	}
}

void sendandreceive(int var, int socket_fd)
{
	char sender[3000];
	char reciever[3000];

	int no_of_bytes;

	if(var==0)
	{
		fgets(sender, 3000, stdin);
		if(strcmp(sender, "exit\n")==0 || strcmp(sender, "quit\n")==0 || strcmp(sender, "bye\n")==0)
		{
			exit(0);
		}

		else
		{
			send(socket_fd, sender, 3000, 0);
		}
	}

	else
	{
		no_of_bytes = recv(socket_fd, reciever, 3000, 0);
		reciever[no_of_bytes] = NULL;
		printf(reciever);
		printf("\n");
		fflush(stdout);
	}
}