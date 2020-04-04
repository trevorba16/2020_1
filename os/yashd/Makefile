all: client server
#client
client: yash.c
	gcc -g -o yash yash.c -lreadline
#server
server: yash.c
	gcc -g -o yashd yashd.c server_thread.c -lpthread