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
#include <pthread.h>



#define PORT 5001
#define NUM_CLIENTS 100
#define NO_CLIENT -1
#define TIMEOUT 2
#define NUM_MESSAGES 10000
#define BUF_SIZE 256
#define BACKLOG 100
#define MESSAGE "HELLO FROM SERVER"

void* sendMessages(void* sockfd){
	int fd = *((int*)sockfd);

	char* buf = MESSAGE;

	for(int n = 0; n < NUM_MESSAGES; n++){
		write(fd, buf, strlen(buf) + 1);
	}

	return NULL;
}

// removes message from read queue
void recvMessage(int fd){
	char buf[BUF_SIZE];

	read(fd, buf, BUF_SIZE);
}

int main(){
	// file descriptor listening for new connections
	int listenFd = socket(PF_INET, SOCK_STREAM, 0);

	if(listenFd < 0){
		perror("socket() failed\n");
		return EXIT_FAILURE;
	}

	// holds address and port of server
	struct sockaddr_in server;

	// zero out sockaddr
	memset(&server, 0, sizeof(struct sockaddr_in));

	// use IPv4
	server.sin_family = AF_INET;
	// allow any ip address
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	// initialize to given port
	server.sin_port = htons(PORT);

	// size of struct for bind()
	int len = sizeof(struct sockaddr_in);

	// bind listening server to address 
	if(bind(listenFd, (struct sockaddr*)&server, len) != 0){
		perror("bind() failed\n");
		return EXIT_FAILURE;
	}

	// set listening socket to listening state
	if(listen(listenFd, BACKLOG) != 0){
		perror("listen() failed\n");
		return EXIT_FAILURE;
	}

	int* clients = calloc(NUM_CLIENTS, sizeof(int));
	for(int n = 0; n < NUM_CLIENTS; n++){
		clients[n] = NO_CLIENT;
	}
	pthread_t* clientThreads = calloc(NUM_CLIENTS, sizeof(pthread_t));

	// structure to hold a set of file descriptors to watch
	fd_set rfds;
	int maxIndex = 0;

	while(1){

		int maxFd = listenFd;

		// set rfds to include no file descriptors
		FD_ZERO(&rfds);

		FD_SET(listenFd, &rfds);

		// add any connected clients
		for(int n = 0; n < NUM_CLIENTS; n++){

			if(clients[n] != NO_CLIENT){
				FD_SET(clients[n], &rfds);
				// max fd is max of highest seen and new fd
				maxFd = maxFd > clients[n] ? maxFd : clients[n];
			}
		}

		// structure to specify TIMEOUT second timeout on select() call
		struct timeval timeout;

		timeout.tv_sec = TIMEOUT;
		timeout.tv_usec = 1000;

		// wait for activity on listening socket, or any active client
		int retval = select(maxFd + 1, &rfds, NULL, NULL, &timeout);

		if(retval == 0){
			//printf("No Activity\n");
			break;
			//return 0;
		}
		else if(retval == -1){
			perror("ERROR Select() failed\n");
			return EXIT_FAILURE;
		}

		if(FD_ISSET(listenFd, &rfds)){
			int index = -1;
			// accept new connection

			for(int n = maxIndex; n < NUM_CLIENTS; n++){
				if(clients[n] == NO_CLIENT){
					index = n;
					maxIndex++;
					break;
				}
			}

			clients[index] = accept(listenFd, NULL, NULL);

			pthread_create(&clientThreads[index], NULL, sendMessages, (void*)&clients[index]);
		}

		for(int n = 0; n < NUM_CLIENTS; n++){
			if(FD_ISSET(clients[n], &rfds)){
				recvMessage(clients[n]);
			}
		}

	}

	for(int n = 0; n < NUM_CLIENTS; n++){
		if(clientThreads[n] != 0){
			pthread_join(clientThreads[n], NULL);
		}
		if(clients[n] != NO_CLIENT){
			close(clients[n]);
		}
	}

	close(listenFd);

	free(clients);
	free(clientThreads);

}



























































