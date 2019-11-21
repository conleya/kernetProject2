all: server client

server: test.c
	gcc test.c -pthread -o serv.out
client: testClient.c
	gcc testClient.c -o client.out
