all: client server
#client
client: yash_client.c
	gcc -g -o yash yash_client.c -lreadline
#server
server: yash_server.c
	gcc -g -o yashd yash_server.c server_thread.c -lpthread