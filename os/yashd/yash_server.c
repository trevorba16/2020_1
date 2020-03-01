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
#define BUFSIZE 2000


struct client_state
{
    int in_use;
    struct job job_array[20];
};

#pragma region FUNCTION DECLARATIONS
int current_thread;
void reusePort(int sock);
void * ServeClient(void * arg);
void cleanup(char *buf);
#pragma endregion

struct client_state client_array[NUM_THREADS];

typedef struct _client_thread_data
{
    int psd;
    int state_array_idx;
    struct sockaddr_in from;
} client_thread_data;

#pragma region INITIALIZATION AND MEMORY
void initializeClientArray()
{
    for (int i = 0; i < NUM_THREADS; i++)
    {
        client_array[i].in_use = 0;
        initializeJobs(client_array[i].job_array);
    }
}
void cleanup(char *buf)
{
    int i;
    for(i=0; i<BUFSIZE; i++) buf[i]='\0';
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
#pragma endregion

int getOpenClientIndex()
{
    for (int i = 0; i < NUM_THREADS; i++)
    {
        if (client_array[i].in_use = 0)
            return i;
    }
}

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
        current_thread = getOpenClientIndex();
        psd  = accept(sd, (struct sockaddr *)&from, &fromlen);
        thr_data[current_thread].from = from;
        thr_data[current_thread].psd = psd;
        thr_data[current_thread].state_array_idx = current_thread;
        pthread_create(&thr[current_thread], NULL, ServeClient, &thr_data[current_thread]);
	}
}

static void sig_child(int signo)
{
    pid_t pid;
    int status;
    // printf("Job completed\n");
    for (int c = 0; c < NUM_THREADS; c++)
    {
        for (int i = 0; i < 20; i++)
        {
            if (client_array[c].job_array[i].job_order != -1 && client_array[c].job_array[i].run_status == 1 && client_array[c].job_array[i].is_background == 1) 
            {
                pid = waitpid(client_array[c].job_array[i].pid, &status, WNOHANG | WUNTRACED);
            
                if (WIFEXITED(status))
                {
                    // printf("job completed %d:%d:%d\n" , job_array[i].job_order, job_array[i].pid, job_array[i].run_status);
                    client_array[c].job_array[i].run_status = 2;
                }
            }
        }
    }
}

void * ServeClient(void * arg) {
    client_thread_data *client_info = (client_thread_data *) arg;
    char buf[BUFSIZE];
    int rc;
    struct  hostent *hp, *gethostbyname();
    char process_output[BUFSIZE] = {0};
    pthread_t job_thread;
    pthread_t recv_thread;

    if (signal(SIGCHLD, sig_child) == SIG_ERR)
        printf("signal(SIGCHLD) error");

    initializeJobs(client_array[client_info->state_array_idx].job_array);



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

    phthread_create(&job_thread, NULL, , NULL);

    for(;;){
        char * hash = "# ";

        if (send(client_info->psd, hash, 2, 0) <0 )
            perror("Couldn't send initial prompt");

        // printf("\n...server is waiting...\n");
        if( (rc=recv(client_info->psd, buf, sizeof(buf), 0)) < 0){
            perror("receiving stream  message");
            exit(-1);
        }
        if (rc > 0){
            buf[rc - 1]='\0';
            char * inString = buf;

            processStarter(inString, client_array[client_info->state_array_idx].job_array, process_output);
            // processStarter(inString, client_array[client_info->state_array_idx].job_array, process_output);

            int output_size = 0;
            for (int i = 0; i < sizeof(process_output); i++)
            {
                if (process_output[i] == '\0')
                {
                    output_size = i;
                    break;
                }
            }
            if (send(client_info->psd, process_output, output_size, 0) <0 )
            // perror("sending stream message");
            rc = 0;
        }
        else {
            // printf("TCP/Client: %s:%d\n", inet_ntoa(client_info->from.sin_addr),
            // ntohs(client_info->from.sin_port));
            // printf("(Name is : %s)\n", hp->h_name);
            // printf("Disconnected..\n");
            close (client_info->psd);
            client_array[client_info->state_array_idx].in_use = 0;
            initializeJobs(client_array[client_info->state_array_idx].job_array);
            exit(0);
        }
        cleanup(process_output);
    }
}

void * ServeClient(void * arg) {
    client_thread_data *client_info = (client_thread_data *) arg;
    char buf[BUFSIZE];
    int rc;
    struct  hostent *hp, *gethostbyname();
    char process_output[BUFSIZE] = {0};
    pthread_t job_thread;
    pthread_t recv_thread;

    if (signal(SIGCHLD, sig_child) == SIG_ERR)
        printf("signal(SIGCHLD) error");

    initializeJobs(client_array[client_info->state_array_idx].job_array);



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

    phthread_create(&job_thread, NULL, , NULL);

    for(;;){
        char * hash = "# ";

        if (send(client_info->psd, hash, 2, 0) <0 )
            perror("Couldn't send initial prompt");

        // printf("\n...server is waiting...\n");
        if( (rc=recv(client_info->psd, buf, sizeof(buf), 0)) < 0){
            perror("receiving stream  message");
            exit(-1);
        }
        if (rc > 0){
            buf[rc - 1]='\0';
            char * inString = buf;

            processStarter(inString, client_array[client_info->state_array_idx].job_array, process_output);
            // processStarter(inString, client_array[client_info->state_array_idx].job_array, process_output);

            int output_size = 0;
            for (int i = 0; i < sizeof(process_output); i++)
            {
                if (process_output[i] == '\0')
                {
                    output_size = i;
                    break;
                }
            }
            if (send(client_info->psd, process_output, output_size, 0) <0 )
            // perror("sending stream message");
            rc = 0;
        }
        else {
            // printf("TCP/Client: %s:%d\n", inet_ntoa(client_info->from.sin_addr),
            // ntohs(client_info->from.sin_port));
            // printf("(Name is : %s)\n", hp->h_name);
            // printf("Disconnected..\n");
            close (client_info->psd);
            client_array[client_info->state_array_idx].in_use = 0;
            initializeJobs(client_array[client_info->state_array_idx].job_array);
            exit(0);
        }
        cleanup(process_output);
    }
}

void * StartJobsFromInput()
{
    for (;;)
    {

        processStarter(inString, client_array[client_info->state_array_idx].job_array, process_output);
    }
}

void * ReceiveUserInput()
{
    for (;;)
    {
        
    }
}
