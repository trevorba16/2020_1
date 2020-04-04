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
#include <sys/un.h>
#include <netdb.h> /* struct hostent */
#include <string.h> /* memset() */
#include <unistd.h> /* close() */
#include <stdlib.h> /* exit() */
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "server_thread.h"
#include <sys/stat.h>
#include <sys/file.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>

#define MAXHOSTNAME 80
#define NUM_THREADS 20
#define BUFSIZE 2000
#define PATHMAX 255

#pragma region STRUCTS
struct client_state
{
    int in_use;
    int running_pid;
    int * pid_point;
    struct job job_array[20];
};
typedef struct _client_thread_data
{
    int psd;
    int state_array_idx;
    struct sockaddr_in from;
    char * inString;
} client_thread_data;

typedef struct _job_thread_data
{
    int state_array_idx;
    int psd;
} job_thread_data;

typedef struct _recv_thread_data
{
    int psd;
    int state_array_idx;
    struct sockaddr_in from;
} recv_thread_data;
#pragma endregion

#pragma region GLOBAL VARIABLES
struct client_state client_array[NUM_THREADS];
pthread_t client_thr[NUM_THREADS];
client_thread_data client_thr_data[NUM_THREADS];
pthread_mutex_t client_array_lock = PTHREAD_MUTEX_INITIALIZER;
recv_thread_data rtd[NUM_THREADS];
job_thread_data jtd[NUM_THREADS];

static char u_server_path[PATHMAX+1] = "/tmp";  /* default */
static char u_socket_path[PATHMAX+1];
static char u_log_path[PATHMAX+1];
static char u_pid_path[PATHMAX+1];

FILE * log_file;
#pragma endregion

#pragma region FUNCTION DECLARATIONS
int current_thread;
void reusePort(int sock);
void * ServeClient(void * arg);
void cleanup(char *buf);
void * StartJobsFromInput(void * arg);
void * ReceiveUserInput(void * arg);
int getOpenClientIndex();
void initializeClientArray();
int getLastProcess(int client_array_idx);
void daemon_init(const char * const path, uint mask);
#pragma endregion

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

int main(int argc, char **argv ) {
    int   sd, psd;
    struct   sockaddr_in server;
    struct  hostent *hp, *gethostbyname();
    struct sockaddr_in from;
    int fromlen;
    int length;
    char ThisHost[80];
    int pn;

    current_thread = 0;

    #pragma region DAEMONIZE
    int  listenfd;
  
  /* Initialize path variables */
    if (argc > 1) 
        strncpy(u_server_path, argv[1], PATHMAX); /* use argv[1] */
    strncat(u_server_path, "/", PATHMAX-strlen(u_server_path));
    strncat(u_server_path, argv[0], PATHMAX-strlen(u_server_path));
    strcpy(u_socket_path, u_server_path);
    strcpy(u_pid_path, u_server_path);
    strncat(u_pid_path, ".pid", PATHMAX-strlen(u_pid_path));
    strcpy(u_log_path, u_server_path);
    strncat(u_log_path, ".log", PATHMAX-strlen(u_log_path));
    
    daemon_init(u_server_path, 0); /* We stay in the u_server_path directory and file
                                        creation is not restricted. */

    #pragma endregion
    
    #pragma region CONNECTION
    /* get TCPServer1 Host information, NAME and INET ADDRESS */
    gethostname(ThisHost, MAXHOSTNAME);
    if  ( (hp = gethostbyname(ThisHost)) == NULL ) {
      fprintf(stderr, "Can't find host %s\n", argv[1]);
      exit(-1);
    }
    bcopy ( hp->h_addr, &(server.sin_addr), hp->h_length);

    
    
    /** Construct name of socket */
    server.sin_family = AF_INET;
    
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    if (argc == 1)
        server.sin_port = htons(3826);  
    else  {
        pn = htons(3826); 
        server.sin_port =  pn;
    }
    
    /** Create socket on which to send  and receive */
    
    sd = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP); 
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
    #pragma endregion
    /** accept TCP connections from clients and fork a process to serve each */
    listen(sd,4);
    fromlen = sizeof(from);
    for(;;)
    {
        current_thread = getOpenClientIndex();
        psd  = accept(sd, (struct sockaddr *)&from, &fromlen);
        client_thr_data[current_thread].from = from;
        client_thr_data[current_thread].psd = psd;
        client_thr_data[current_thread].state_array_idx = current_thread;
        pthread_create(&client_thr[current_thread], NULL, ServeClient, &client_thr_data[current_thread]);
	}
}

int getOpenClientIndex()
{
    for (int i = 0; i < NUM_THREADS; i++)
    {
        if (client_array[i].in_use == 0)
            return i;
    }
}
static void sig_child(int signo)
{
    pid_t pid;
    int status;
    for (int c = 0; c < NUM_THREADS; c++)
    {
        for (int i = 0; i < 20; i++)
        {
            if (client_array[c].job_array[i].job_order != -1 && client_array[c].job_array[i].run_status == 1 && client_array[c].job_array[i].is_background == 1) 
            {
                pid = waitpid(client_array[c].job_array[i].pid, &status, WNOHANG | WUNTRACED);
            
                if (WIFEXITED(status))
                {
                    client_array[c].job_array[i].run_status = 2;
                }
            }
        }
    }
}

#pragma region CLIENT HANDLING
void * ServeClient(void * arg) {
    client_thread_data *client_info = (client_thread_data *) arg;
    pthread_t job_thread;
    pthread_t recv_thread;
    
    struct  hostent *hp, *gethostbyname();
    char inString[2000] = {0};

    client_thr_data[client_info->state_array_idx].inString = inString;
    client_array[client_info->state_array_idx].running_pid = -1;
    client_array[client_info->state_array_idx].pid_point = &client_array[client_info->state_array_idx].running_pid;

    if (signal(SIGCHLD, sig_child) == SIG_ERR)
        printf("signal(SIGCHLD) error");

    initializeJobs(client_array[client_info->state_array_idx].job_array);

    if ((hp = gethostbyaddr((char *)&client_info->from.sin_addr.s_addr,
			    sizeof(client_info->from.sin_addr.s_addr),AF_INET)) == NULL)
	{
        fprintf(stderr, "Can't find host %s\n", inet_ntoa(client_info->from.sin_addr));
    }
    else
    {
    }


    char * hash = "# ";
    if (send(client_info->psd, hash, 2, 0) <0 )
        perror("Couldn't send initial prompt");

    struct sockaddr_in from = client_info->from;
    int psd = client_info->psd;
    int array_idx = client_info->state_array_idx;

    /**  get data from  client and send it back */
    jtd[array_idx].state_array_idx = array_idx;
    jtd[array_idx].psd = psd;
    pthread_create(&job_thread, NULL, StartJobsFromInput, &jtd[array_idx]);


    rtd[array_idx].from = from;
    rtd[array_idx].state_array_idx = array_idx;
    rtd[array_idx].psd = psd;
    pthread_create(&recv_thread, NULL, ReceiveUserInput, &rtd[array_idx]);
}
#pragma endregion

#pragma region START PROCESS
void * StartJobsFromInput(void * arg)
{
    
    job_thread_data *thread_info = (job_thread_data *) arg;
    
    int array_idx = thread_info->state_array_idx;
    char process_output[BUFSIZE] = {0};

    for (;;)
    {
        pthread_mutex_lock(&client_array_lock);
        char c = client_thr_data[array_idx].inString[0];
        pthread_mutex_unlock(&client_array_lock);
        if (c == '\0')
        {
        }
        else 
        {
            int hours, minutes, seconds, day, month, year;
            time_t now;
            time(&now);
            struct tm *local = localtime(&now);

            hours = local->tm_hour;      	// get hours since midnight (0-23)
            minutes = local->tm_min;     	// get minutes passed after the hour (0-59)
            seconds = local->tm_sec;     	// get seconds passed after minute (0-59)

            day = local->tm_mday;        	// get day of month (1 to 31)
            month = local->tm_mon + 1;   	// get month of year (0 to 11)
            year = local->tm_year + 1900;	// get year since 1900

            char mon[4] = {0};

            mon[0] = ctime(&now)[4];
            mon[1] = ctime(&now)[5];
            mon[2] = ctime(&now)[6];
            mon[3] = '\0';

            log_file = fopen(u_log_path, "aw");
            fprintf(log_file, "%s %d %02d:%02d:%02d yashd[%s:%d]: %s\n", 
                        mon, day, hours, minutes, seconds, 
                        inet_ntoa(client_thr_data[array_idx].from.sin_addr), ntohs(client_thr_data[array_idx].from.sin_port),
                        client_thr_data[array_idx].inString);

            fclose(log_file);

            processStarter(client_thr_data[array_idx].inString, client_array[thread_info->state_array_idx].job_array, process_output, client_array[array_idx].pid_point);

            pthread_mutex_lock(&client_array_lock);
            cleanup(client_thr_data[array_idx].inString);
            pthread_mutex_unlock(&client_array_lock);

            // printf("%d\n", 23);
            int output_size = 0;
            strcat(process_output, "\n# ");
            for (int i = 0; i < sizeof(process_output); i++)
            {
                if (process_output[i] == '\0')
                {
                    output_size = i;
                    break;
                }
            }

            if (send(thread_info->psd, process_output, output_size, 0) <0 )
                perror("sending stream message");
            
            cleanup(process_output);
            
        }
    }
}
#pragma endregion

#pragma region RECEIVE INPUT
void * ReceiveUserInput(void * arg)
{
    recv_thread_data * thread_info = (recv_thread_data *) arg;
    int array_idx = thread_info->state_array_idx;
    int rc = -1;
    char buf[BUFSIZE];

    int psd = thread_info->psd;

    struct  hostent *hp, *gethostbyname();
    if ((hp = gethostbyaddr((char *)&thread_info->from.sin_addr.s_addr,
			    sizeof(thread_info->from.sin_addr.s_addr),AF_INET)) == NULL)
	{
        fprintf(stderr, "Can't find host %s\n", inet_ntoa(thread_info->from.sin_addr));
    }
    else
    {
    }

    for(;;){
        if( (rc=recv(psd, buf, sizeof(buf), 0)) < 0){
            perror("receiving stream  message");
            exit(-1);
        }
        if (rc > 0){
            pthread_mutex_lock(&client_array_lock);
            if (buf[4] == '\n')
            {
                client_thr_data[array_idx].inString[0] = '\n';
                client_thr_data[array_idx].inString[1] = '\0';
            }
            else if (buf[0] == 'C' && buf[1] == 'M' && buf[2] == 'D')
            {
                buf[rc - 1]='\0';

                int c = 0;
   
                while (c < rc) {
                    client_thr_data[array_idx].inString[c] = buf[4+c];
                    c++;
                }
                client_thr_data[array_idx].inString[c] = '\0';
            }
            else if (buf[0] == 'C' && buf[1] == 'T' && buf[2] == 'L')
            {
                buf[rc - 1]='\0';

                if (buf[4] == 'C') 
                {                    
                    if (client_array[array_idx].running_pid != 0)
                    {
                        kill(client_array[array_idx].running_pid, SIGINT);

                        client_array[array_idx].running_pid = 0;

                        int kill_job_idx = -1;
                        for (int i = 0; i < 20; i++)
                        {
                            if (client_array[array_idx].job_array[kill_job_idx].pid == client_array[array_idx].running_pid)
                                kill_job_idx = i;
                        }

                        if (kill_job_idx != -1)
                        {
                            memset(job_array[kill_job_idx].args, 0, 2000);
                            client_array[array_idx].job_array[kill_job_idx].job_order = -1;
                            client_array[array_idx].job_array[kill_job_idx].pid = -1;
                            client_array[array_idx].job_array[kill_job_idx].run_status = -1;
                            client_array[array_idx].job_array[kill_job_idx].is_background = -1;
                        }
                        
                    }
                    else 
                    {
                        client_thr_data[array_idx].inString[0] = ' ';
                    }
                    client_thr_data[array_idx].inString[1] = '\0';
                }
                else if (buf[4] == 'Z') 
                {
                    if (client_array[array_idx].running_pid != 0)
                    {
                        kill(client_array[array_idx].running_pid, SIGTSTP);

                        client_array[array_idx].running_pid = 0;
                    }
                    
                }
                else if (buf[4] == 'D') 
                {
                    exit(0);
                }
            }
            pthread_mutex_unlock(&client_array_lock);
            rc = 0;
            cleanup(buf);
            
        }
        else {
            
        }
    }
}
int getLastProcess(int client_array_idx)
{
    
    int max_process = -1;
    int max_job_order = -1;
    int max_index = -1;

    
    for (int i = 0; i < 20; i++)
     {
        if (client_array[client_array_idx].job_array[i].job_order != -1 
            && client_array[client_array_idx].job_array[i].pid != -1 
            && client_array[client_array_idx].job_array[i].run_status != -1) 
        {
            struct job j = client_array[client_array_idx].job_array[i];
            
            if (j.job_order > max_job_order && j.run_status == 1)
            {
                max_job_order = j.job_order;
                max_index = i;
            }
        }
    }
    return max_index;
}
#pragma endregion

#pragma region DAEMON INIT
/**
 * @brief  If we are waiting reading from a pipe and
 *  the interlocutor dies abruptly (say because
 *  of ^C or kill -9), then we receive a SIGPIPE
 *  signal. Here we handle that.
 */
void sig_pipe(int n) 
{
   perror("Broken pipe signal");
}


/**
 * @brief Handler for SIGCHLD signal 
 */
void sig_chld(int n)
{
  int status;

  fprintf(stderr, "Child terminated\n");
  wait(&status); /* So no zombies */
}

/**
 * @brief Initializes the current program as a daemon, by changing working 
 *  directory, umask, and eliminating control terminal,
 *  setting signal handlers, saving pid, making sure that only
 *  one daemon is running. Modified from R.Stevens.
 * @param[in] path is where the daemon eventually operates
 * @param[in] mask is the umask typically set to 0
 */
void daemon_init(const char * const path, uint mask)
{
  pid_t pid;
  char buff[256];
  
  int fd;
  int k;

  /* put server in background (with init as parent) */
  if ( ( pid = fork() ) < 0 ) {
    perror("daemon_init: cannot fork");
    exit(0);
  } else if (pid > 0) /* The parent */
    exit(0);

  /* the child */

  /* Close all file descriptors that are open */
  for (k = getdtablesize()-1; k>0; k--)
      close(k);

  /* Redirecting stdin and stdout to /dev/null */
  if ( (fd = open("/dev/null", O_RDWR)) < 0) {
    perror("Open");
    exit(0);
  }
  dup2(fd, STDIN_FILENO);      /* detach stdin */
  dup2(fd, STDOUT_FILENO);     /* detach stdout */
  close (fd);
  /* From this point on printf and scanf have no effect */

  /* Redirecting stderr to u_log_path */
  log_file = fopen(u_log_path, "aw"); /* attach stderr to u_log_path */
  
  /* From this point on printing to stderr will go to /tmp/u-echod.log */

  /* Establish handlers for signals */
  if ( signal(SIGCHLD, sig_chld) < 0 ) {
    perror("Signal SIGCHLD");
    exit(1);
  }
  if ( signal(SIGPIPE, sig_pipe) < 0 ) {
    perror("Signal SIGPIPE");
    exit(1);
  }

  /* Change directory to specified directory */
  chdir(path); 

  /* Set umask to mask (usually 0) */
  umask(mask); 
  
  /* Detach controlling terminal by becoming sesion leader */
  setsid();

  /* Put self in a new process group */
  pid = getpid();
  setpgrp(); /* GPI: modified for linux */

  /* Make sure only one server is running */
  if ( ( k = open(u_pid_path, O_RDWR | O_CREAT, 0666) ) < 0 )
  {
    perror("Cannot start yashd daemon, other instance already running");
    exit(1);
  }  
  if ( lockf(k, F_TLOCK, 0) != 0)
    exit(0);

  /* Save server's pid without closing file (so lock remains)*/
  sprintf(buff, "%6d", pid);
  write(k, buff, strlen(buff));

  return;
}
#pragma endregion