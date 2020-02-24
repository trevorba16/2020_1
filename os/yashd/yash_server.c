/**
 * @file TCPServer-ex2.c 
 * @brief The program creates a TCP socket in
 * the inet domain and listens for connections from TCPClients, accept clients
 * into private sockets, and fork an echo process to ``serve'' the
 * client.  If [port] is not specified, the program uses any available
 * port.  
 * Run as: 
 *     TCPServer-ex2 <port>
 */
/*
NAME:        
SYNOPSIS:    TCPServer [port]

DESCRIPTION:  

*/
#include <stdio.h>
/* socket(), bind(), recv, send */
#include <sys/types.h>
#include <sys/socket.h> /* sockaddr_in */
#include <netinet/in.h> /* inet_addr() */
#include <arpa/inet.h>
#include <netdb.h> /* struct hostent */
#include <string.h> /* memset() */
#include <unistd.h> /* close() */
#include <stdlib.h> /* exit() */
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "server_thread.h"


#define MAXHOSTNAME 80
#define NUM_THREADS 20

#pragma region FUNCTION DECLARATIONS
int current_thread;
void reusePort(int sock);
void * ServeClient(void * arg);
#pragma endregion

typedef struct _client_thread_data
{
    int psd;
    struct sockaddr_in from;
} client_thread_data;

int main(int argc, char **argv ) {
    int   sd, psd;
    struct   sockaddr_in server;
    struct  hostent *hp, *gethostbyname();
    struct sockaddr_in from;
    int fromlen;
    int length;
    char ThisHost[80];
    int pn;
    pthread_t thr[NUM_THREADS];
    client_thread_data thr_data[NUM_THREADS];

    current_thread = 0;
    
    /* get TCPServer1 Host information, NAME and INET ADDRESS */
    gethostname(ThisHost, MAXHOSTNAME);
    /* OR strcpy(ThisHost,"localhost"); */
    
    printf("----TCP/Server running at host NAME: %s\n", ThisHost);
    if  ( (hp = gethostbyname(ThisHost)) == NULL ) {
      fprintf(stderr, "Can't find host %s\n", argv[1]);
      exit(-1);
    }
    bcopy ( hp->h_addr, &(server.sin_addr), hp->h_length);
    printf("    (TCP/Server INET ADDRESS is: %s )\n", inet_ntoa(server.sin_addr));

    
    
    /** Construct name of socket */
    server.sin_family = AF_INET;
    /* OR server.sin_family = hp->h_addrtype; */
    
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    if (argc == 1)
        server.sin_port = htons(3826);  
    else  {
        pn = htons(3826); 
        server.sin_port =  pn;
    }
    
    /** Create socket on which to send  and receive */
    
    sd = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP); 
    /* OR sd = socket (hp->h_addrtype,SOCK_STREAM,0); */
    if (sd<0) {
        perror("opening stream socket");
        exit(-1);
    }
    /** this allow the server to re-start quickly instead of waiting
	for TIME_WAIT which can be as large as 2 minutes */
    reusePort(sd);
    if ( bind( sd, (struct sockaddr *) &server, sizeof(server) ) < 0 ) {
        close(sd);
        perror("binding name to stream socket");
        exit(-1);
    }
    
    /** get port information and  prints it out */
    length = sizeof(server);
    if ( getsockname (sd, (struct sockaddr *)&server,&length) ) {
        perror("getting socket name");
        exit(0);
    }
    printf("Server Port is: %d\n", ntohs(server.sin_port));
    
    /** accept TCP connections from clients and fork a process to serve each */
    listen(sd,4);
    fromlen = sizeof(from);
    for(;;){
        psd  = accept(sd, (struct sockaddr *)&from, &fromlen);
        thr_data[current_thread].from = from;
        thr_data[current_thread].psd = psd;
        pthread_create(&thr[current_thread], NULL, ServeClient, &thr_data[current_thread]);
	}
}

void * ServeClient(void * arg) {
    client_thread_data *client_info = (client_thread_data *) arg;
    char buf[512];
    int rc;
    struct  hostent *hp, *gethostbyname();
    struct job client_jobs[20];
    char process_output[4096] = {0};

    initializeJobs(client_jobs);

    // printf("Serving %s:%d\n", inet_ntoa(client_info->from.sin_addr), ntohs(client_info->from.sin_port));
    if ((hp = gethostbyaddr((char *)&client_info->from.sin_addr.s_addr,
			    sizeof(client_info->from.sin_addr.s_addr),AF_INET)) == NULL)
	{
        fprintf(stderr, "Can't find host %s\n", inet_ntoa(client_info->from.sin_addr));
    }
    else
    {
	    // printf("(Name is : %s)\n", hp->h_name);
    }
    /**  get data from  client and send it back */
    for(;;){
        // printf("\n...server is waiting...\n");
        if((rc=recv(client_info->psd, buf, sizeof(buf), 0)) < 0)
        {
            perror("receiving stream  message");
            exit(-1);
        }
        if (rc > 0)
        {
            buf[rc - 1]='\0';
            char * inString = buf;
            processStarter(inString, client_jobs, process_output);
            if (send(client_info->psd, process_output, sizeof(process_output), 0) <0 )
		        perror("sending stream message");
        }
        else 
        { 
            close (client_info->psd);
        }
    }
}
void reusePort(int s)
{
    int one=1;
    
    if ( setsockopt(s,SOL_SOCKET,SO_REUSEADDR,(char *) &one,sizeof(one)) == -1 )
	{
	    printf("error in setsockopt,SO_REUSEPORT \n");
	    exit(-1);
	}
}