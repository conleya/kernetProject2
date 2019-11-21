#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <netdb.h>

#define MESSAGE "HELLO FROM CLIENT"
#define NUM_MESSAGES 10000
#define BUF_SIZE 256
#define PORT 5001

int main(){

	// create socket, IPv4, tcp
	int sdTCP = socket(PF_INET, SOCK_STREAM, 0);

	if(sdTCP == -1){
		perror("ERROR: Failed to create socket\n");
		return EXIT_FAILURE;
	}

	struct sockaddr_in server;
	/* from stack overflow:

			"So the correct thing to do is to use AF_INET in your 
			struct sockaddr_in and PF_INET in your call to socket().
			But practically speaking, you can use AF_INET everywhere."

	*/
	server.sin_family = AF_INET;
	// connect to localhost, inet_addr removes periods and turns string into desired format
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	// set server port to which we will connect
	server.sin_port = htons(PORT);

	socklen_t len = sizeof(server);

	// connect
	if(connect(sdTCP, (struct sockaddr*)&server, len) != 0){
		perror("ERROR: connect() failed\n");
		return EXIT_FAILURE;
	}

	// fd set for the one fd, to poll
	fd_set rfds;

	char* readBuf[BUF_SIZE];
	char* buf = MESSAGE; 

	for(int n = 0; n < NUM_MESSAGES; n++){
		FD_ZERO(&rfds);

		FD_SET(sdTCP, &rfds);

		// structure to specify TIMEOUT second timeout on select() call
		struct timeval timeout;

		timeout.tv_sec = 0;
		timeout.tv_usec = 0;

		select(sdTCP + 1, &rfds, NULL, NULL, &timeout);


		write(sdTCP, buf, strlen(buf) + 1);
		if(FD_ISSET(sdTCP, &rfds)){
			read(sdTCP, readBuf, BUF_SIZE);
		}
	}
	int bytes = 1;
	while(bytes != 0){
		bytes = read(sdTCP, readBuf, BUF_SIZE);
	}

	close(sdTCP);


}