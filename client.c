#include<stdio.h>
#include<string.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<stdlib.h>
#include<stdbool.h>
#include<netdb.h>
#include<unistd.h>    //write

#define INITSIZE 20

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

	int socket_desc;
	struct sockaddr_in server;
	struct addrinfo hints, *res;
	
	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1) {
		fprintf(stderr,"Could not create socket\n");
		return 1;
	}

	char *argumentpointer = argv[1];
	bool justhead = false;

	if (argc > 2) {
		for (int i = 0; i < argc; i++) {
			if (strncmp(argv[i], "-I",2)) {
				justhead = true;
			}
			argumentpointer = argv[argc-1];
		}
	}
	
	int fulllength = strlen(argumentpointer);
	char address[fulllength];
	memset(&address[0], 0, fulllength);
	strncpy(address, argumentpointer, fulllength);
	
	if (fulllength == 0) {
		return 1;
	}
	
	char addressurl[200];
	memset(&addressurl[0], 0, 200);
	char portnum[10];
	memset(&portnum[0], 0, 10);
	char filepath[200];
	memset(&filepath[0], 0, 200);
	int position = 0;

	if (address[position] == 'h') {
		//strncpy(addressurl,&address[position],1);
		position++;
		while (address[position] != 'w') {
			//strncat(addressurl,&address[position],1);
			position++;
		}
		strncat(addressurl,&address[position],1);
		position++;
	} else if (address[position] == 'w') {
		strncpy(addressurl,&address[position],1);
		position++;
	}
	
	while (position != fulllength && address[position] != ':' && address[position] != '/') {
		strncat(addressurl,&address[position],1);
		position++;
	}
	
	if (position != fulllength && address[position] == ':' && address[position+1] != '/') {
		position++;
		strncpy(portnum,&address[position],1);
		position++;
		while (position != fulllength && address[position] != '/') {
			strncat(portnum,&address[position],1);
			position++;
		}
	} else if (position != fulllength && (address[position] == '/' || (address[position] == ':' && address[position+1] == '/'))) {
		strncpy(portnum, "80", 4);
		if (address[position] == ':') {
			position++;
		}
	}

	if (position != fulllength && position+1 != fulllength && address[position] == '/') {
		position++;
		strncpy(filepath,&address[position],1);
		position++;
		for (int i = position; i < fulllength; i++) {
			strncat(filepath,&address[i],1);
		}
	} else {
		printf("No file specified\n");
		return 1;
	}

	//printf("FULL INPUT: %s\n",address);
	//printf("Address: %s\n",addressurl);
	//printf("Port Number: %s\n",portnum);
	//printf("File Path: %s\n",filepath);

	// Stuff for getaddrinfo
	int rv;  
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // use AF_INET6 to force IPv6
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(addressurl,"http",&hints,&res)) != 0) {
		fprintf(stderr,"Could not resolve address\n");
		return 1;
	}
	struct sockaddr_in *saddr = (struct sockaddr_in*)res->ai_addr;
	
	// Replace with first command line parameter
	//server.sin_addr.s_addr = inet_addr("216.58.194.67");
	server.sin_addr.s_addr = inet_addr(inet_ntoa(saddr->sin_addr));
	server.sin_family = AF_INET;
	server.sin_port = htons(strtol(portnum, NULL, 10));

	//Connect to remote server
	if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0) {
		fprintf(stderr,"Connect Error\n");
		return 1;
	}

	//puts("Connected\n");

	write(socket_desc, "GET /", 5);
	//printf("GET /");
	//printf("%s",filepath);
	//printf(" HTTP/1.1\n");
	//printf("filepath size: %i\n",strlen(filepath));
	write(socket_desc, filepath, strlen(filepath));
	write(socket_desc, " HTTP/1.1\r\n", 11);

	//write(socket_desc, "User-Agent: curl/7.38.0\r\n", 25);
	write(socket_desc, "Host: ", 6);
	write(socket_desc, addressurl, strlen(addressurl));
	write(socket_desc, "\r\n", 2);

	write(socket_desc, "Connection: close\r\n", 19);

	write(socket_desc, "\r\n", 2);

	int read_size = 0;
	int len = 0;
	int msglen; 
	int linelen;
	char **client_request;
	int lineCounter = 0;
	//int msgCounter = 0

	client_request = malloc(INITSIZE*sizeof(char*));
	if (client_request == NULL) {
		fprintf(stderr,"Malloc Failed\n");
		return 0;
	}

	allocatestr(lineCounter, client_request);

	msglen = INITSIZE;
	linelen = INITSIZE;

	while ((read_size = read(socket_desc,&client_request[lineCounter][len],1)) == 1) {

		len++;
		
		if (client_request[lineCounter][len-1] == '\n') {

			memset(&client_request[lineCounter][len], 0, msglen-len);

			lineCounter++;
			msglen = INITSIZE;
			len = 0;

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
				freestr(lineCounter, client_request);
				return 0;

			}

			memset(&client_request[lineCounter][len], 0, msglen-len);
		}
		
	}

	if (justhead == true) {
		for (int i = 0; i < lineCounter; i++) {
			printf("%s",client_request[i]);
		}
	} else {

		char *contentlength;
		int filelength = 0;

		for (int i = 0; i < lineCounter; i++) {
			contentlength = strstr(client_request[i],"Content-Length");
			if (contentlength != NULL) {
				char *pch;

				pch = strtok(client_request[i]," ");
				pch = strtok(NULL, " ");
			
				filelength = strtol(pch, NULL, 10);
		
				break;
			}
		}

		char *filemessage;
		filemessage = malloc(filelength * sizeof(char));
		memset(&filemessage[0], 0, filelength);
		if (filemessage == NULL) {
			return 1;
		}

		if (filelength != 0) {
			while(1) {
				if ((read_size = recv(socket_desc,&filemessage[0],filelength,0)) > 0) {
					
					printf("%s",filemessage);
					memset(filemessage,0, strlen(filemessage));
				} else {
					//memset(&filemessage[filelength], 0, 1);
					break;
				}
			}
		} else {
			int currentlength = 0;
			filemessage = realloc(filemessage, INITSIZE * sizeof(char));

			while ((read_size = read(socket_desc,&filemessage[currentlength],INITSIZE)) > 0) {
				currentlength = currentlength + INITSIZE;
				filemessage = realloc(filemessage, (INITSIZE + currentlength)*sizeof(char));
			}
			//printf("Length: %i\n",strlen(filemessage));
			memset(&filemessage[strlen(filemessage)], 0, currentlength + INITSIZE - strlen(filemessage));
			printf("%s",filemessage);
		}

		free(filemessage);
	}

	return 0;
}
