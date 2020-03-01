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

#define MAXHOSTNAME 80
#define NUM_THREADS 20
#define BUFSIZE 2000
#define	MAXLINE	4096
#define MAX_JOBS 20


#pragma region CONNECTION
#pragma region FUNCTION DECLARATIONS
int current_thread;
void reusePort(int sock);
void * ServeClient(void * arg);
void cleanup(char *buf);
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
    char buf[BUFSIZE];
    int rc;
    struct  hostent *hp, *gethostbyname();
    struct job client_jobs[20];
    char process_output[BUFSIZE] = {0};

    initializeJobs(client_jobs);


    if (signal(SIGCHLD, sig_child) == SIG_ERR)
        printf("signal(SIGCHLD) error");

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

            // printf("======================================================\n");
            // printf("=====================BEFORE PROCESS===================\n");
            // printf("======================================================\n");

            // for (int i = 0; i < 20; i++) 
            // {
            //     struct job j = client_jobs[i];
            //     printf("order: %d\n", j.job_order);
            //     printf("args:  %s\n", j.args);
            //     printf("run: %d\n", j.run_status);
            // }

            processStarter(inString, process_output);

            // printf("======================================================\n");
            // printf("=====================AFTER PROCESS====================\n");
            // printf("======================================================\n");

            // for (int i = 0; i < 20; i++) 
            // {
            //     struct job j = client_jobs[i];
            //     printf("order: %d\n", j.job_order);
            //     printf("args:  %s\n", j.args);
            //     printf("run: %d\n", j.run_status);
            // }

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
            exit(0);
        }
        cleanup(process_output);
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

#pragma region HANDLE THREAD

#pragma region GLOBAL VARIABLES
struct job
{
    int pid;
    int run_status; // 1 is running, 0 is stopped // 2 is done
    int job_order;
    int is_background; 
    char args[2000];
};
int pid_ch1, pid_ch2, ppid, job_num, status;
struct job job_array[20];
char *inString;
#pragma endregion

#pragma region FUNCTION DECLARATIONS
void setjobAsBackground(int pid);
void removeJobFromLog(int rem_index);
static void sig_ignore(int signo);
static void sig_int(int signo);
static void sig_tstp(int signo);
static void sig_int(int signo);
int addJobToLog(int is_background);
void executeChildProcess(char** args, int argc, int input_index, int output_index, int error_index, int background_index);
void processSingleCommand(char** args, int argc, int input_index, int output_index, int error_index, int background_index, char* output_content); 
void processPipeCommand(char** init_args, int argc, int pipe_index, int background_index, char * output_content);
int getMostRecentBackground(int is_background);
void processForegroundCommand(char * output_content);
void processBackgroundCommand(char * output_content);
void processJobsCommand(char * output_content);
void printJob(int index, int is_bg, char * output_content);
void findAndPrintCompletedJobs(char * output_content);
void processStarter(char * inString, char * process_output);
void initializeJobs();
// void copy_arrays(struct job main_arr[], struct job to_copy_arr[]);
#pragma endregion

#pragma region SIGNAL HANDLING
static void sig_ignore(int signo) 
{
}

static void sig_int(int signo) 
{
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (job_array[i].pid == pid_ch1) 
        {
            removeJobFromLog(i);
            break;
        }
    }
    kill(pid_ch1,SIGINT);
}
static void sig_tstp(int signo) 
{
    kill(pid_ch1,SIGTSTP);
    for (int i = 0; i < MAX_JOBS; i++) 
    {
        if (job_array[i].pid == pid_ch1) 
        {
            job_array[i].run_status = 0;
            break;
        }
    }
}
static void sig_child(int signo)
{
    pid_t pid;
    int status;
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (job_array[i].job_order != -1 && job_array[i].run_status == 1 && job_array[i].is_background == 1) 
        {
            pid = waitpid(job_array[i].pid, &status, WNOHANG | WUNTRACED);
        
            if (WIFEXITED(status))
            {
                // printf("job completed %d:%d:%d\n" , job_array[i].job_order, job_array[i].pid, job_array[i].run_status);
                job_array[i].run_status = 2;
            }
        }
    }
}
#pragma endregion

#pragma region JOB LOG MANAGEMENT
int addJobToLog(int is_background) 
{
    struct job new_job;

    new_job.pid = pid_ch1;
    new_job.run_status = 1;
    new_job.job_order = ++job_num;
    // printf("process_num: %d\n", pid_ch1);
    new_job.is_background = is_background;
    strcpy(new_job.args, inString);
    for (int i = 0; i < MAX_JOBS; i++) 
    {
        if (job_array[i].job_order == -1 
            && job_array[i].pid == -1 
            && job_array[i].run_status == -1) 
        {
            job_array[i] = new_job;
            return i;
        }
    }
    // printf("added to job log\n");
}

void removeJobFromLog(int rem_index)
{
    memset(job_array[rem_index].args, 0, 2000);
    job_array[rem_index].job_order = -1;
    job_array[rem_index].pid = -1;
    job_array[rem_index].run_status = -1;
    job_array[rem_index].is_background = -1;
}

void setjobAsBackground(int pid) 
{
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (job_array[i].pid == pid)
            job_array[i].is_background = 1;
    }
}

int getMostRecentBackground(int is_background)
{
    // printf("Starting getMostRecentBackground()\n");
    int max_process = -1;
    int max_job_order = -1;
    int max_index = -1;

    
    for (int i = 0; i < MAX_JOBS; i++)
     {
        if (job_array[i].job_order != -1 
            && job_array[i].pid != -1 
            && job_array[i].run_status != -1) 
        {
            struct job j = job_array[i];
            // printf("Made job\n");
            // printf("run_status: %d\n", j.run_status);
            // printf("jobOrder: %d\n", j.job_order);
            // printf("pid: %d\n", j.pid);
            if (is_background == 1)
            {
                if (j.run_status == 0 && j.job_order > max_job_order)
                {
                    max_job_order = j.job_order;
                    max_index = i;
                }
            }
            else 
            {
                if (j.job_order > max_job_order)
                {
                    max_job_order = j.job_order;
                    max_index = i;
                }
            }
        }
    }
    return max_index;
}
#pragma endregion

#pragma region DEFAULT COMMAND EXECUTION
void executeChildProcess(char** args, int argc, int input_index, int output_index, int error_index, int background_index) 
{
    args[argc] = NULL;

    pid_t pid = getpid();
    setpgid(pid, pid);

    if (input_index != -1) {

        args[input_index] = NULL;

        char* input_string = args[input_index + 1];
        int ofd;

        ofd = open(input_string, 0644);
        dup2(ofd, STDIN_FILENO);
    }

    if (output_index != -1) {

        args[output_index] = NULL;

        char* output_string = args[output_index + 1];
        int ofd;
        ofd = creat(output_string, 0644);
        dup2(ofd, STDOUT_FILENO);
    }

    if (error_index != -1) {

        args[error_index] = NULL;

        char* error_string = args[error_index + 1];
        int ofd;
        ofd = creat(error_string, 0644);
        dup2(ofd, STDERR_FILENO);
    }

    if (background_index != -1) 
    {
        args[background_index] = NULL;
    }
    // printf("Child: %d\n", 1);
    // printf("args: %s\n", args[0]);
    execvp(args[0], args);
    // printf("Child: %d\n", 2);
    exit(0);
}

void processSingleCommand(char** args, int argc, int input_index, int output_index, int error_index, int background_index, char* output_content) 
{

    int filedes[2];

    int is_background = 0;
    if (background_index != -1)
        is_background = 1;

    if (pipe(filedes) == -1) {
        perror("pipe had an error");
    }

    pid_ch1 = fork();
    if (pid_ch1 > 0) 
    {
        // Parent
        close(filedes[1]);
        int job_idx = addJobToLog(is_background);
        if (background_index != -1) {
            kill(pid_ch1, SIGTSTP);
            kill(pid_ch1, SIGCONT);
        }
        else {
            // printf("%d\n", 1);
            setpgid(pid_ch1, pid_ch1);

            // if (signal(SIGINT, sig_int) == SIG_ERR)
            //     printf("signal(SIGINT) error");
            // if (signal(SIGTSTP, sig_tstp) == SIG_ERR)
            //     printf("signal(SIGTSTP) error");
            // if (signal(SIGCHLD, sig_child) == SIG_ERR)
            //     printf("signal(SIGCHLD) error");

            // printf("%d\n", 2);
            int count = 0;
            while (count < 1) 
            {
                // printf("%d\n", 3);
                ppid = waitpid(pid_ch1, &status, WUNTRACED | WNOHANG);
                // printf("waiting for %d\n", pid_ch1);
                
                if (ppid == -1) 
                {
                    perror("waitpid");
                    exit(EXIT_FAILURE);
                }

                if (WIFEXITED(status)) 
                {
                    removeJobFromLog(job_idx);
                    //printf("child %d exited, status=%d\n", ppid, WEXITSTATUS(status));
                    count++;
                } 
                else if (WIFSIGNALED(status)) 
                {
                    //printf("child %d killed by signal %d\n", ppid, WTERMSIG(status));
                    count++;
                } 
                else if (WIFSTOPPED(status)) {
                    count++;
                } else if (WIFCONTINUED(status)) {
                }
            }
            char buffer[BUFSIZE];
            while (1) 
            {
                // printf("Looking for output\n");
                ssize_t count = read(filedes[0], buffer, sizeof(buffer));
                // printf("Count from read: %zu\n", count);
                if (count == -1) {
                    perror("waitpid");
                    exit(EXIT_FAILURE);
                } else if (count == 0) {
                    break;
                } else {
                    // printf("Got this in parent: %s\n", buffer);
                    for (int i = 0; i < count; i++)
                    {
                        output_content[i] = buffer[i];
                    }
                }
            }
        }
    }
    else if (pid_ch1 == 0) {
        // Child
        close(filedes[0]);
        dup2(filedes[1], STDOUT_FILENO);
        close(filedes[1]);
        executeChildProcess(args, argc, input_index, output_index, error_index, background_index);
    }
}

void processPipeCommand(char** init_args, int argc, int pipe_index, int background_index, char * output_content) 
{

    char **args_left = malloc((argc) * sizeof(char *));
    char **args_right = malloc((argc) * sizeof(char *));

    int left_input = -1;
    int left_output = -1;
    int left_error = -1;

    int right_input = -1;
    int right_output = -1;
    int right_error = -1;
    int right_background = -1;

    #pragma region Split Commands
    for (int i = 0; i < pipe_index; i++) {
        args_left[i] = init_args[i];

        if (args_left[i][0] == '>') {
            left_output = i;
        }
        else if (args_left[i][0] == '<') {
            left_input = i;
        }
        else if (args_left[i][0] == '2' && args_left[i][1] == '>') {
            left_error = i;
        }
    }
    args_left[pipe_index] = NULL;


    for (int i = 0; i < (argc - pipe_index) - 1; i++) {
        args_right[i] = init_args[pipe_index + i + 1];

          if (args_right[i][0] == '>') {
            right_output = i;
        }
        else if (args_right[i][0] == '<') {
            right_input = i;
        }
        else if (args_right[i][0] == '2' && args_right[i][1] == '>') {
            right_error = i;
        }
        else if (args_right[i][0] == '&')
        {
            right_background = i;
        }
    }
    args_right[(argc - pipe_index) - 1] = NULL;
    #pragma endregion

    int pipeLtoR[2];
    char buf;
    
    if (pipe(pipeLtoR) == -1) {
        perror("pipe had an error");
    }
    pid_ch1 = fork();
    if (pid_ch1 == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    if (pid_ch1 == 0) 
    {    
        /*
        *
        * WRITE SIDE / Child 1
        * 
        * */
        close(pipeLtoR[0]); 
        dup2(pipeLtoR[1], STDOUT_FILENO); 
        close(pipeLtoR[1]); 

        if (left_input != -1) {

            args_left[left_input] = NULL;

            char* input_string = args_left[left_input + 1];
            int ofd;

            ofd = open(input_string, 0644);
            dup2(ofd, STDIN_FILENO);
        }

        if (left_output != -1) {

            args_left[left_output] = NULL;
            
            char* output_string = args_left[left_output + 1];
            int ofd;
            ofd = creat(output_string, 0644);
            dup2(ofd, STDOUT_FILENO);
        }

        if (left_error != -1) {

            args_left[left_error] = NULL;

            char* error_string = args_left[left_error + 1];
            int ofd;
            ofd = creat(error_string, 0644);
            dup2(ofd, STDERR_FILENO);
        }

        execvp(args_left[0], args_left);
        exit(0);
    } 
    else 
    {   
        /*
        *
        * READ SIDE AND PARENT
        * 
        * */  
        int pipeRtoP[2];

        if (pipe(pipeRtoP) == -1) {
            perror("pipe had an error");
        }      

        pid_ch2 = fork();

        if (pid_ch2 == -1) {
            perror("Child 2 had a problem");
            exit(EXIT_FAILURE);
        }
        if (pid_ch2 == 0) {
            // READ CHILD AND OUTPUT TO PARENT PIPE
            setpgid(pid_ch1, pid_ch1);


            close(pipeRtoP[0]); // read end
            close(pipeLtoR[1]); // write end
            
            dup2(pipeLtoR[0], STDIN_FILENO); 
            dup2(pipeRtoP[1], STDOUT_FILENO);

            close(pipeLtoR[0]); 
            close(pipeRtoP[1]);
 

            if (right_input != -1) {

                args_right[right_input] = NULL;

                char* input_string = args_right[right_input + 1];
                int ofd;

                ofd = open(input_string, 0644);
                dup2(ofd, STDIN_FILENO);
            }

            if (right_output != -1) {

                args_right[right_output] = NULL;

                char* output_string = args_right[right_output + 1];
                int ofd;
                ofd = creat(output_string, 0644);
                dup2(ofd, STDOUT_FILENO);
            }

            if (right_error != -1) {

                args_right[right_error] = NULL;

                char* error_string = args_right[right_error + 1];
                int ofd;
                ofd = creat(error_string, 0644);
                dup2(ofd, STDERR_FILENO);
            }

            if (background_index != -1) 
            {
                args_right[background_index] = NULL;
            }
            // printf("executing: %s\n", args_right[0]);
            execvp(args_right[0], args_right);
            exit(0);
        }
        else {

            // Parent

            close(pipeRtoP[1]);
            close(pipeLtoR[1]);

            setpgid(pid_ch1, pid_ch1);
            setpgid(pid_ch2, pid_ch1);

            int is_background = 0;
            if (background_index != -1)
            {
                is_background = 1;
            }

            int job_idx = addJobToLog(is_background);

            if (background_index != -1) 
            {
                kill(pid_ch1, SIGTSTP);
                kill(pid_ch1, SIGCONT);
            }
            else 
            {
                int count = 0;
                while (count < 1) 
                {
                    ppid = waitpid(pid_ch1, &status, WUNTRACED | WNOHANG);
                    
                    if (ppid == -1) 
                    {
                        perror("waitpid");
                        exit(EXIT_FAILURE);
                    }

                    if (WIFEXITED(status)) 
                    {
                        removeJobFromLog(job_idx);
                        count++;
                    } 
                    else if (WIFSIGNALED(status)) 
                    {
                        count++;
                    } 
                    else if (WIFSTOPPED(status)) {
                        count++;
                    } else if (WIFCONTINUED(status)) {
                    }
                }
                char buffer[BUFSIZE];
                while (1) 
                {
                    // printf("Looking for output\n");
                    ssize_t count = read(pipeRtoP[0], buffer, sizeof(buffer));
                    // printf("Count from read: %zu\n", count);
                    if (count == -1) {
                        perror("waitpid");
                        exit(EXIT_FAILURE);
                    } else if (count == 0) {
                        break;
                    } else {
                        // printf("Got this in parent: %s\n", buffer);
                        for (int i = 0; i < count; i++)
                        {
                            output_content[i] = buffer[i];
                        }
                    }
                    // printf("output: %s\n", output_content);
                }
            }
        }
    }
}
#pragma endregion

#pragma region CUSTOM JOB COMMANDS
void processForegroundCommand(char * output_content) 
{
    int max_index = getMostRecentBackground(1);

    int max_pid = job_array[max_index].pid;
    setpgid(max_pid, max_pid);
    int count = 0;
    int first_stop = 0;
    // if (signal(SIGINT, sig_int) == SIG_ERR)
    //     printf("signal(SIGINT) error");
    // if (signal(SIGTSTP, sig_tstp) == SIG_ERR)
    //     printf("signal(SIGTSTP) error");
    // if (signal(SIGCHLD, sig_child) == SIG_ERR)
        // printf("signal(SIGCHLD) error");
    while (count < 2) 
    {
        ppid = waitpid(max_pid, &status, WNOHANG | WUNTRACED | WCONTINUED);
        // printf("waiting\n");
        
        if (ppid == -1) 
        {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }

        if (WIFEXITED(status)) 
        {
            removeJobFromLog(max_index);
            count++;
        } 
        else if (WIFSIGNALED(status)) 
        {
            //printf("child %d killed by signal %d\n", ppid, WTERMSIG(status));
            count++;
        } 
        else if (WIFSTOPPED(status)) {
            if (first_stop == 0) {
                pid_ch1 = max_pid;
                for (int i = 0; i < sizeof(job_array[max_index].args); i++)
                {
                    output_content[i] = job_array[max_index].args[i];
                }
                kill(max_pid, SIGCONT);
                job_array[max_index].run_status = 1;
                first_stop++;
            }
            count++;
        } else if (WIFCONTINUED(status)) {
            //printf("Continuing %d\n",ppid);
        }
    }
}

void processBackgroundCommand(char * output_content)
{
    int max_index = getMostRecentBackground(1);

    int max_pid = job_array[max_index].pid;

    printJob(max_index, 1, output_content);
    
    setpgid(max_pid, max_pid);

    kill(max_pid, SIGCONT);

    job_array[max_index].run_status = 1;
    job_array[max_index].is_background = 1;
}

void processJobsCommand(char * output_content)
{
    findAndPrintCompletedJobs(output_content);
    int index_order[20];
    for (int i = 0; i < MAX_JOBS; i++) 
    {
       for (int j = i + 1; j < MAX_JOBS; ++j)
        {
            if (job_array[i].job_order > job_array[j].job_order) 
            {
                struct job temp_job =  job_array[i];
                job_array[i] = job_array[j];
                job_array[j] = temp_job;
            }
        }
    }
    for (int i = 0; i < MAX_JOBS; i++) 
    {
        if (job_array[i].job_order != -1)
        {
            printJob(i, 0, output_content);
        }
    }
}
#pragma endregion

#pragma region PRINTING
void printJob(int index, int is_bg, char * output_content) 
{
    int rec_idx = getMostRecentBackground(0);
    strcat(output_content, "[");
    char buffer[3];
    sprintf(buffer, "%d", job_array[index].job_order);
    strcat(output_content, buffer);
    strcat(output_content, "]");
    // printf("[%d]", job_array[index].job_order);
    if (index == rec_idx) 
    {
        strcat(output_content, "+ ");
        // printf("+ ");
    }
    else
    {
        strcat(output_content, "- ");
        // printf("- ");
    }
    if (is_bg == 0)
    {
        if (job_array[index].run_status == 0) 
        {
            strcat(output_content, "Stopped\t\t");
            // printf("Stopped\t\t");
        }
        else if (job_array[index].run_status == 1) 
        {
            strcat(output_content, "Running\t\t");
            // printf("Running\t\t");
        }
        else if (job_array[index].run_status == 2) 
        {
            strcat(output_content, "Done\t\t");
            // printf("Done\t\t");
        }
    }
    strcat(output_content, job_array[index].args);
    // printf("%s", job_array[index].args);
    if (is_bg == 1)
    {
        strcat(output_content, " &");
        // printf(" &");
    }
    strcat(output_content, "\n");
    // printf("\n");
    
}

void findAndPrintCompletedJobs(char * output_content) 
{
    int max_order = 0;
    for (int i = 0; i < MAX_JOBS; i++) 
    {
        // printf("%d:%d:%d\n", job_array[i].job_order, job_array[i].pid, job_array[i].run_status);
        if (job_array[i].run_status == 2) 
        {
            
            printJob(i, 0, output_content);
            removeJobFromLog(i);
        }
        else if (job_array[i].job_order > max_order) 
        {
            // printf("Reassign to %d\n", job_array[i].job_order);
            max_order = job_array[i].job_order;
        }
    }
    // printf("max_order: %d\n", max_order);
    job_num = max_order;
}
#pragma endregion

void initializeJobs()
{
    for (int i = 0; i < MAX_JOBS; i++) 
    {
        removeJobFromLog(i);
    }
    job_num = 0;
}

// void copy_arrays(struct job main_arr[], struct job to_copy_arr[])
// {
//     for (int i = 0; i < MAX_JOBS; i++)
//     {
//         to_copy_arr[i] = main_arr[i];
//     }
// }

void processStarter(char * client_inString, char * process_output)
{
    inString = client_inString;
    // printf("got the following to execute: %s\n", inString);
    
    // copy_arrays(client_jobs, job_array);

    // printf("======================================================\n");
    // printf("=====================INSIDE PROCESS===================\n");
    // printf("======================================================\n");

    //     for (int i = 0; i < 20; i++) 
    // {
    //     printf("Process-------------------\n");
    //     struct job j = client_jobs[i];
    //     printf("order: %d\n", j.job_order);
    //     printf("args:  %s\n", j.args);
    //     printf("run: %d\n", j.run_status);
    //     printf("Client--------------------\n");
    //     struct job j2 = client_jobs[i];
    //     printf("order: %d\n", j2.job_order);
    //     printf("args:  %s\n", j2.args);
    //     printf("run: %d\n", j2.run_status);
    // }
    // printf("arrays copied\n");
    // if (signal(SIGINT, SIG_DFL) == SIG_ERR)
    //     printf("signal(SIGINT) error");
    // if (signal(SIGTSTP, SIG_DFL) == SIG_ERR)
    //     printf("signal(SIGTSTP) error");

    pid_ch1 = -1;
    pid_ch2 = -1;
    int input_length = strlen(inString);
    int pipe_index = -1;
    int input_index = -1;
    int output_index = -1;
    int error_index = -1;
    int background_index = -1;

    char argv[100][50]; 
    int j,ctr;
    

    #pragma region PARSE INSTRING
    j=0; ctr=0;
    for(int i=0;i<=(strlen(inString));i++) {

        if(inString[i] == '|') 
        {
            pipe_index = ctr;
        }
        if(inString[i] == '<') 
        {
            input_index = ctr;
        }
        if(inString[i] == '>') 
        {
            if (inString[i-1] == '2') 
            {
                error_index = ctr;
            }
            else  {
                output_index = ctr;
            } 
        }
        if(inString[i] == '&') 
        {
            background_index = ctr;
        }
        
        // if space or NULL found, assign NULL into newString[ctr]
        if(inString[i]==' '||inString[i]=='\0') 
        {
            argv[ctr][j]='\0';
            ctr++;  //for next word
            j=0;    //for next word, init index to 0
        }
        else 
        {
            argv[ctr][j]=inString[i];
            j++;
        }
    }
    #pragma endregion
    
    #pragma region CHECK FOR CUSTOM COMMANDS
    int foreground = 0;
    int background = 0;
    int jobs = 0;

    char **args = malloc((ctr  + 1)* sizeof(char *));

    for (int i = 0; i < ctr; i++) 
    {
        
        args[i] = argv[i];
        if (strcmp(argv[i],"fg") == 0) 
        {
            foreground = 1;
        }
        else if (strcmp(argv[i],"bg") == 0)
        {
            background = 1;
        }
        else if (strcmp(argv[i],"jobs") == 0)
        {
            jobs = 1;
        }
    }
    #pragma endregion
    
    #pragma region  EXECUTE COMMANDS
    if (foreground == 1) 
    {
        processForegroundCommand(process_output);
    }
    else if (background == 1)
    {
        processBackgroundCommand(process_output);
    }
    else if (jobs == 1)
    {
        processJobsCommand(process_output);
    }
    else 
    {
        if (pipe_index != -1) 
        {
            processPipeCommand(args, ctr, pipe_index, background_index, process_output);
        }
        else 
        {
            processSingleCommand(args, ctr, input_index, output_index, error_index, background_index, process_output);
        }
    }
    #pragma endregion
    // if (signal(SIGINT, sig_ignore) == SIG_ERR)
    //     printf("signal(SIGINT) error");
    // if (signal(SIGTSTP, sig_ignore) == SIG_ERR)
    //     printf("signal(SIGTSTP) error");
    findAndPrintCompletedJobs(process_output);
    // copy_arrays(job_array, client_jobs);

    // printf("======================================================\n");
    // printf("=====================COMPLETED PROCESS===================\n");
    // printf("======================================================\n");

    // for (int i = 0; i < 20; i++) 
    // {
    //     printf("Process-------------------\n");
    //     // struct job j = client_jobs[i];
    //     // printf("order: %d\n", j.job_order);
    //     // printf("args:  %s\n", j.args);
    //     // printf("run: %d\n", j.run_status);
    //     printf("Client--------------------\n");
    //     struct job j2 = job_array[i];
    //     printf("order: %d\n", j2.job_order);
    //     printf("args:  %s\n", j2.args);
    //     printf("run: %d\n", j2.run_status);
    // }
    
}
#pragma endregion