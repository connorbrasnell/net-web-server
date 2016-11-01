#include<stdio.h>
#include<string.h>    
#include<stdlib.h>    
#include<sys/socket.h>
#include<arpa/inet.h> 
#include<unistd.h>   
#include<pthread.h> 
 
FILE *f;
int counter = 0;
pthread_mutex_t lock_x;

void *connection_handler(void *);
 
int main(int argc , char *argv[]) {
    
	int socket_desc , new_socket , c , *new_sock;
	struct sockaddr_in server , client;

	f = fopen(argv[2], "w+");

	if (f == NULL) {
	    fprintf(stderr,"Error opening file!\n");
	}

	// Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);

	if (socket_desc == -1) {
		fprintf(stderr,"Could not create socket");
	}

	// Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(strtol(argv[1], NULL, 10));

	// Bind
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0) {
		fprintf(stderr,"Bind Failed\n");
		return 1;
	}

	puts("Bind Done");

	// Listen
	listen(socket_desc , 3);

	// Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);

	while( (new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) ) {
		puts("Connection accepted");
		 
		pthread_t sniffer_thread;
		new_sock = malloc(1);
		*new_sock = new_socket;
		 
		if( pthread_create( &sniffer_thread , NULL ,  &connection_handler , (void*) new_sock) < 0) {
			perror("could not create thread");
			return 1;
		}

		puts("Handler assigned");
	}

	if (new_socket < 0) {
		fprintf(stderr,"Accept Failed\n");
	}

	return 0;
}

void *connection_handler(void *socket_desc) {

	int sock = *(int*)socket_desc;
	int read_size;
	int len = 0;
	int msglen; 
	char *client_message;

	client_message = malloc(20*sizeof(char));

	if (client_message == NULL) {
		fprintf(stderr,"Malloc Failed\n");
		free(socket_desc);
		return 0;
	}

	msglen = 20;

	while ((read_size = read(sock,&client_message[len],1)) == 1) {
		
		len++;
		
		if (client_message[len-1] == '\n') {

			pthread_mutex_lock(&lock_x);
				fprintf(f, "%i %s", counter, client_message);
				counter++;
			pthread_mutex_unlock(&lock_x);

			memset(&client_message[0], 0, msglen);
			len = 0;
			msglen = 20;

		} else if (len == msglen) {

			msglen = msglen + 20;
			client_message = realloc(client_message, msglen*sizeof(char));
			if (client_message == NULL) {
				fprintf(stderr,"Malloc Failed\n");
				free(socket_desc);
				free(client_message);
				return 0;

			}

			memset(&client_message[len], 0, msglen-len);
		}
	}

	free(client_message);

	if (read_size == 0) {
		puts("Client Disconnected");
	} else if(read_size == -1) {
		fprintf(stderr,"Recv Failed\n");
	}

	fflush(f);
	 
	close(sock);

	// Free the socket pointer
	free(socket_desc);

	pthread_exit(NULL);

	return 0;
}
