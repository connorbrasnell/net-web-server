#include<stdio.h>
#include<string.h>    
#include<stdlib.h>    
#include<sys/socket.h>
#include<arpa/inet.h> 
#include<unistd.h>   
#include<pthread.h> 
#include<time.h>
 
FILE *f;
pthread_mutex_t lock_x;
#define INITSIZE 20

void *connection_handler(void *);

/*
 * Function:  allocatestr
 * --------------------
 * Allocates space for 20 sets of 20 characters
 *
 *  start: Initial index of dystrings (pointer array) to start from
 *	   (Does not have to start from index 0)
 *  client_request: List of requests
 */
void allocatestr(int start, char** client_request) {

	int limit = start + INITSIZE;

	for (int i = start; i < limit; i++) {
    		client_request[i] = malloc(INITSIZE * sizeof(char));
		// If malloc cannot assign enough memory, quit the program
		if (client_request[i] == NULL) {
			exit(1);
		}
	}
}

/*
 * Function:  freestr
 * --------------------
 * Free all allocated memory
 *
 *  ctr: Amount of strings
 */
void freestr(int ctr, char** client_request) {

	int freeamount = 0;

	// If amount of strings is a multiple of 20, or one less, 
	// have allocated 20 extra char array pointers
	if ( ((ctr % 20) == 0) || ((ctr % 20) == 19) ) {
		freeamount = (20-(ctr % 20)) + ctr + 20;
	} else {
		freeamount = (20-(ctr % 20)) + ctr;
	}

	for (int i = 0; i < freeamount; i++) {
		free(client_request[i]);
	}

	free(client_request);
}
 
int main(int argc , char *argv[]) {
    
	int socket_desc , new_socket , c , *new_sock;
	struct sockaddr_in server , client;

	f = fopen("/tmp/logfile.txt", "w+");

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
	server.sin_port = htons(8088);

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
	int linelen;
	char **client_request;
	int lineCounter = 0;
	//int msgCounter = 0

	client_request = malloc(INITSIZE*sizeof(char*));
	if (client_request == NULL) {
		fprintf(stderr,"Malloc Failed\n");
		free(socket_desc);
		return 0;
	}

	allocatestr(lineCounter, client_request);

	msglen = INITSIZE;
	linelen = INITSIZE;

	while ((read_size = read(sock,&client_request[lineCounter][len],1)) == 1) {

		len++;
		
		if (client_request[lineCounter][len-1] == '\n') {

			lineCounter++;
			msglen = INITSIZE;
			len = 0;

			//printf("Line Inside: %i: %s",lineCounter,client_request[lineCounter-1]);

			if (client_request[lineCounter-1][0] == '\r' && client_request[lineCounter-1][1] == '\n') {
				break;
			}			

			if (lineCounter == linelen) {
				allocatestr(lineCounter, client_request);
			}

		} else if (len == msglen) {

			msglen = msglen + INITSIZE;

			client_request[lineCounter] = realloc(client_request[lineCounter],(msglen * sizeof(char)));
			if (client_request[lineCounter] == NULL) {
				fprintf(stderr,"Malloc Failed\n");
				free(socket_desc);
				freestr(lineCounter, client_request);
				return 0;

			}

			memset(&client_request[lineCounter][len], 0, msglen-len);
		}
		
	}

	// Above loop always reads a single extra character when done, so scrap that with memset
	// memset(&client_request[lineCounter][len], 0, 1);

	/*for (int i = 0; i < lineCounter; i++) {
		printf("Line %i: %s",i+1,client_request[i]);
	}*/

	char *pch;

	pch = strtok(client_request[0]," ");
	char *operation = pch;
	
	pch = strtok(NULL, " ");
	if (pch[0] == '/') { // Get rid of initial / in url
		pch++;
	}
	char *filepath = pch;

	char *operationget = strstr(client_request[0],"GET");
	char *operationhead = strstr(client_request[0],"HEAD");

	if (operationget != NULL || operationhead != NULL) {
		
		printf("Operation: %s\n",operation);
		printf("File Path: %s\n",filepath);

		char datebuf[1000];
		time_t now = time(0);
		struct tm tm = *gmtime(&now);
		strftime(datebuf, sizeof datebuf, "%a, %d %b %Y %H:%M:%S %Z", &tm);
		datebuf[strlen(datebuf)] = '\n';
		

		write(sock, "HTTP/1.1 200 OK\r\n", 16);
		
		write(sock, "Date: ", 6);
		write(sock, datebuf, strlen(datebuf));
		write(sock, "\r\n", 2);

		write(sock, "\r\n", 2);
		//write(sock, "<html><head></head><body>Test</body></html>\r\n", 45);

		if (operationget != NULL) {

			FILE *fp;
			fp = fopen(filepath,"r");
		
			char filebuffer[50];

			while(1) {

				if (feof(fp)) {
					break;
				}
			
				if (fgets(filebuffer,50,fp) != NULL) {
					write(sock,filebuffer,strlen(filebuffer));
				} else {
					break;
				}
			}

			fclose(fp);
		}
	}

	freestr(lineCounter, client_request);

	if (read_size == 0) {
		puts("Client Disconnected");
	} else if(read_size == -1) {
		fprintf(stderr,"Recv Failed\n");
	}

	//fflush(f);
	//close(f);
	 
	close(sock);

	// Free the socket pointer
	free(socket_desc);

	pthread_exit(NULL);

	return 0;
}
