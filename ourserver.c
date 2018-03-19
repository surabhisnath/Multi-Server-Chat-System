#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 5000
#define MAX_CLIENTS 100

static int NUM_CLIENTS = 0;	

static int client_id = 1;

struct client
{
	struct sockaddr_in address;
	int fd;
	int client_id;	
}obj;	



struct client *arrofclients[MAX_CLIENTS];

void add_client(struct client *cl);

void remove_client(int client_id);

void send_all_exceptme(char *message, int my_id);

void send_all(char *message);

void send_private(char *message, int client_id);

void *manage_clients(void *arg);


int main(int argc, char *argv[])
{
	struct sockaddr_in serveraddr;	
	struct sockaddr_in clientaddr;	


	pthread_t thread_id;
	

	int sock_fd = 0;			
	sock_fd=socket(AF_INET,SOCK_STREAM,0);
	//if((sock_fd=socket(AF_INET,SOCK_STREAM,0))==0)
	// {
	// 	printf("\n Error,Socket went wrong \n");
	// 	return -1;
	// }

	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(PORT);

	signal(SIGPIPE, SIG_IGN);


	if(bind(sock_fd,(struct sockaddr*)&serveraddr,sizeof(serveraddr))<0)
	{
		 printf("Error in binding\n");
		 return 1;
	}

	if(listen(sock_fd,10)<0)
	{
		printf("Error in listening\n");
		return 1;
	}


	printf("Server started\n");


	int fd = 0;
	
	while(1)
	{
		socklen_t value = sizeof(clientaddr);
		fd = accept(sock_fd, (struct sockaddr*)&clientaddr, &value);
		
		
		if(NUM_CLIENTS+1>MAX_CLIENTS)
		{
			NUM_CLIENTS=MAX_CLIENTS;
			close(fd);
			continue;
		}

		struct client *new_client = (struct client *)malloc(sizeof(struct client));	

		new_client->fd = fd;
		new_client->client_id = client_id++;
		new_client->address = clientaddr; 
		

		add_client(new_client);
		pthread_create(&thread_id, NULL, &manage_clients, (void*)new_client);

		sleep(2);
	}
}


void add_client(struct client *cl)
{
	for(int i=0; i<MAX_CLIENTS; i++)
	{
		if(!arrofclients[i])
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
		if(arrofclients[i] && arrofclients[i]->client_id == client_id)
		{
			arrofclients[i] = NULL;
			break;
		}
	}

	return;
}


void send_all(char *message)
{
	for(int i=0; i<MAX_CLIENTS; i++)
	{
		if(arrofclients[i])
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
		if(arrofclients[i] && arrofclients[i]->client_id!=my_id)
		{
			write(arrofclients[i]->fd, message, strlen(message));
		}
	}	
}


void send_private(char *message, int client_id)
{
	for(int i=0; i<MAX_CLIENTS; i++)
	{
		if(arrofclients[i] && arrofclients[i]->client_id==client_id)
		{
			write(arrofclients[i]->fd, message, strlen(message));
		}
	}
}



void *manage_clients(void *arg)
{

	char inp[3000];
	char outp[3000];
	int inplen;

	NUM_CLIENTS++;
	struct client *client_obj = (struct client *)arg;


	printf("Accepted client %d\n",client_obj->client_id);

	sprintf(outp, "Client %d joined", client_obj->client_id);
	send_all(outp);
	

	while((inplen = read(client_obj->fd, inp, sizeof(inp)-1))>0)
	{

		inp[inplen] = '\0';
		
		int j=0;
		while(inp[j] != '\0')
		{
			if(inp[j] == '\r' || inp[j] == '\n')
			{
				inp[j] = '\0';
			}
		
			j++;
		}

		if(!strlen(inp))
			continue;

		if(inp[0]!='$')
		{
			sprintf(outp, "Client %d sent message: %s", client_obj->client_id, inp);
			send_all_exceptme(outp, client_obj->client_id);
		}

		else
		{
			char *message_tosend;
			message_tosend = strtok(inp," ");
			message_tosend = strtok(NULL, " ");

			int id_private = atoi(message_tosend);

			char send[3000];
			message_tosend = strtok(NULL, " ");
			while(message_tosend != NULL)
			{
				strcat(send, message_tosend);
				strcat(send," ");
				message_tosend = strtok(NULL," ");
			}


			char tosend[3000];
			
			sprintf(tosend,"Client %d sent private message: %s", client_obj->client_id, send);
			send_private(tosend,id_private);
		}
	}

	close(client_obj->fd);

	int i = client_obj->client_id;
	

	printf("Client %d exited\n",i);
	
	sprintf(outp,"Client %d left",i);
	send_all(outp);

	remove_client(i);

	free(client_obj);

	NUM_CLIENTS = NUM_CLIENTS-1;

	pthread_detach(pthread_self());
	
	return NULL;
}



