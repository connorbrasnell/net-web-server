#include<stdio.h>
#include<string.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h> //inet_addr
#include<stdlib.h>
#include<stdbool.h>
#include<netdb.h>
#include<unistd.h>    //write

int main(int argc , char *argv[])
{
	int socket_desc;
	struct sockaddr_in server;
	struct addrinfo hints, *res;
	
	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1) {
		fprintf(stderr,"Could not create socket\n");
		return 1;
	}

	// Stuff for getaddrinfo
	int rv;  
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET; // use AF_INET6 to force IPv6
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo("127.0.0.1","http",&hints,&res)) != 0) {
		fprintf(stderr,"Could not resolve address\n");
		return 1;
	}
	struct sockaddr_in *saddr = (struct sockaddr_in*)res->ai_addr;
	
	// Replace with first command line parameter
	//server.sin_addr.s_addr = inet_addr("216.58.194.67");
	server.sin_addr.s_addr = inet_addr(inet_ntoa(saddr->sin_addr));
	server.sin_family = AF_INET;
	server.sin_port = htons(8088); //strtol(argv[2], NULL, 10));

	//Connect to remote server
	if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0) {
		fprintf(stderr,"Connect Error\n");
		return 1;
	}

	puts("Connected\n");

	// Grab the strings (and print them for now)	
	//char buf[20];
	char *strinputs;
	int stdamount = 20;
	//int newsize = 20;
	//int len = 0;

	/*while(true) {

		strinputs = malloc(stdamount * sizeof(char));
		if (strinputs == NULL) {
			fprintf(stderr,"Malloc Failed\n");
			return 1;
		}
		
		fgets(buf,20,stdin);

		strncpy(strinputs,buf,20);
		len = strlen(strinputs);

		while ((strinputs[len-1] != '\n') && !feof(stdin)) { // Run until end of line, i.e. \n char

			newsize = newsize + stdamount;
			// Increase amount of memory for string
			strinputs = realloc(strinputs,(newsize * sizeof(char)));
			// If realloc cannot assign enough memory, quit the program
			if (strinputs == NULL) {
				fprintf(stderr,"Realloc Failed\n");
				return 1;
			}			
			fgets(buf,20,stdin);
			
			strncat(strinputs,buf,20);
			len = strlen(strinputs);
		}

		if (!feof(stdin)) {
			write(socket_desc , strinputs , strlen(strinputs));
		}
		
		free(strinputs);
		newsize = 20;

		if (feof(stdin)) {
			break;
		}
	}*/

	strinputs = malloc(stdamount * sizeof(char));
	if (strinputs == NULL) {
		fprintf(stderr,"Malloc Failed\n");
		return 1;
	}

	strncpy(strinputs,"hello\n",5);

	printf("%s",strinputs);

	write(socket_desc,strinputs,strlen(strinputs));

	return 0;
}
