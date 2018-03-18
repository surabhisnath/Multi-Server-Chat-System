// #include<stdio.h>
// #include<string.h>
// #include<signal.h>
// #include<stdlib.h>
// #include<netinet/in.h>
// #include<pthread.h>
// #include<sys/types.h>
// #include<sys/socket.h>


#include <sys/socket.h>//
#include <netinet/in.h>//
#include <arpa/inet.h>
#include <stdio.h>//
#include <stdlib.h>//
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>//
#include <sys/types.h>//
#include <signal.h>//

#define PORT 5000
#define MAX_CLIENTS 100

struct client
{
	struct sockaddr_in address;	//addr
	int fd;	//connfd
	int client_id;	//uid
}obj;	//client_t

int NUM_CLIENTS = 0;	//cli_count

int client_id = 1;		//uid



struct client *arrofclients[MAX_CLIENTS];

void add_client(struct client *cl);	//Queue_add

void remove_client(int client_id);	//Queue_delete

void send_all_exceptme(char *message, int my_id);		//send_message

void send_all(char *message);		//send_message_all

void send_private(char *message, int client_id);	//send_message_client

void *manage_clients(void *arg);	//handle_client


int main(int argc, char *argv[])
{
	struct sockaddr_in serveraddr;	//serv_addr
	struct sockaddr_in clientaddr;	//cli_addr

	pthread_t thread_id;		//tid

	int sock_fd = 0;			//listenfb
	sock_fd=socket(AF_INET,SOCK_STREAM,0);
	//if((sock_fd=socket(AF_INET,SOCK_STREAM,0))==0)
	// {
	// 	perror("\n Error,Socket went wrong \n");
	// 	return -1;
	// }

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(PORT);

	signal(SIGPIPE, SIG_IGN);


	if(bind(sock_fd,(struct sockaddr*)&serveraddr,sizeof(serveraddr))<0)
	{
		 perror("\n error,Binding went wrong \n");
		 return 1;
	}

	if(listen(sock_fd,5)<0)
	{
		perror("\n Listening went wrong! \n");
		return 1;
	}


	printf("Server started\n");


	int fd = 0;			//connfd
	
	while(1)
	{
		fd = accept(sock_fd, (struct sockaddr*)&clientaddr, sizeof(clientaddr));
		// {
		// 	perror("\n Accept went wrong! \n");
		// 	return -1;
		// }

		NUM_CLIENTS = NUM_CLIENTS + 1;

		if(NUM_CLIENTS>MAX_CLIENTS)
		{
			NUM_CLIENTS=MAX_CLIENTS;
			close(fd);
			continue;
		}

		struct client *new_client = (struct client *)malloc(sizeof(obj));	//cli
		//new_client = {.address = clientaddr, .fd = fd, .client_id = NUM_CLIENTS+1};
		new_client->fd = fd;
		new_client->client_id = client_id++;
		new_client->address = clientaddr; 
		
		//sprintf("%d\n", new_client.client_id);

		add_client(new_client);
		pthread_create(&thread_id, NULL, &manage_clients, (void*)new_client);

		sleep(1);
	}

	return 0;
}


void add_client(struct client *cl)
{
	printf("In add_client\n");
	for(int i=0; i<MAX_CLIENTS; i++)
	{
		if(arrofclients[i]!=NULL)
		{
			arrofclients[i] = cl;
			break;
		}
	}

	return;
}

void remove_client(int client_id)
{
	for(int i=0; i<MAX_CLIENTS; i++)
	{
		if(arrofclients[i]!=NULL)
		{
			if(arrofclients[i]->client_id == client_id)
			{
				arrofclients[i] = NULL;
				break;
			}
		}
	}

	return;
}


void send_all(char *message)
{
	printf("%s\n","I am being executed");
	for(int i=0; i<MAX_CLIENTS; i++)
	{
		if(arrofclients[i]!=NULL)
		{
			//send(arrofclients[i]->fd, message, strlen(message), NULL);
			write(arrofclients[i]->fd, message, strlen(message));
		}
	}
}


void send_all_exceptme(char *message, int my_id)
{
	for(int i=0; i<MAX_CLIENTS; i++)
	{
		if(arrofclients[i]!=NULL && arrofclients[i]->client_id!=my_id)
		{
			send(arrofclients[i]->fd, message, strlen(message), NULL);
		}
	}	
}



void *manage_clients(void *arg)
{

	printf("In manage clients");
	struct client *client_obj = (struct client *)arg;

	char inp[3000];
	char outp[3000];
	int inplen;

	printf("Accepted client %d\n",client_obj->client_id);

	sprintf(outp, "Client %d joined\n", client_obj->client_id);
	send_all(outp);

	printf("before while\n");
	
	printf(inplen);
	//int v = read(client_obj->fd, inp, sizeof(inp)-1);
	//printf("v ");
	//printf("%d",v);

	//sleep(10);

	while((inplen = recv(client_obj->fd, inp, sizeof(inp)-1), NULL)>0)
	{
		printf("%s\n","hi");
		inp[inplen] = '\0';
		//empty output buffer

		if(inplen==0)
			continue;

		if(inp[0]!='\\')
		{
			sprintf(outp, "Client %d sent message: %s\n", client_obj->client_id, inp);
			send_all_exceptme(outp, client_obj->client_id);
		}

		// else
		// {

		// }
	}

	printf("return val ");
	printf("%d",inplen);


	close(client_obj->fd);

	int i = client_obj->client_id;
	remove_client(i);

	printf("Client %d exited\n",i);
	
	sprintf(outp,"Client %d left\n",i);
	send_all(outp);

	NUM_CLIENTS = NUM_CLIENTS-1;

	pthread_detach(pthread_self());
	free(client_obj);

	return NULL;
}



