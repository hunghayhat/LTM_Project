all: server client

server: server.o storage.o
	gcc server.o storage.o -o server
client: client.o
	gcc client.o -o client

client.o : client.c
	gcc -c client.c
server.o : server.c
	gcc -c server.c 

clean :
	rm *.o