#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <fcntl.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "server_thread.h"

#define	MAXLINE	4096
#define MAX_JOBS 20
#define BUFSIZE 2000
// #pragma region GLOBAL VARIABLES
// struct job
// {
//     int pid;
//     int run_status; // 1 is running, 0 is stopped // 2 is done
//     int job_order;
//     int is_background; 
//     char args[2000];
// };
// int pid_ch1, pid_ch2, ppid, job_num, status;
// struct job job_array[20];
// char *inString;
// #pragma endregion

// #pragma region FUNCTION DECLARATIONS
// void setjobAsBackground(int pid);
// void removeJobFromLog(int rem_index);
// static void sig_ignore(int signo);
// static void sig_int(int signo);
// static void sig_tstp(int signo);
// static void sig_int(int signo);
// int addJobToLog(int is_background);
// void executeChildProcess(char** args, int argc, int input_index, int output_index, int error_index, int background_index);
// void processSingleCommand(char** args, int argc, int input_index, int output_index, int error_index, int background_index);
// void processPipeCommand(char** init_args, int argc, int pipe_index, int background_index);
// int getMostRecentBackground(int is_background);
// void processForegroundCommand();
// void processBackgroundCommand();
// void processJobsCommand();
// void printJob(int index, int is_bg);
// void findAndPrintCompletedJobs();
// #pragma endregion

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

void initializeJobs(struct job client_jobs[])
{
    for (int i = 0; i < MAX_JOBS; i++) 
    {
        removeJobFromLog(i);
    }
    job_num = 0;
    copy_arrays(job_array, client_jobs);
}

void copy_arrays(struct job main_arr[], struct job to_copy_arr[])
{
    for (int i = 0; i < MAX_JOBS; i++)
    {
        to_copy_arr[i] = main_arr[i];
    }
}

void processStarter(char * client_inString, struct job client_jobs[], char * process_output)
{
    inString = client_inString;
    // printf("got the following to execute: %s\n", inString);
    
    copy_arrays(client_jobs, job_array);

    printf("======================================================\n");
    printf("=====================INSIDE PROCESS===================\n");
    printf("======================================================\n");

        for (int i = 0; i < 20; i++) 
    {
        printf("Process-------------------\n");
        struct job j = client_jobs[i];
        printf("order: %d\n", j.job_order);
        printf("args:  %s\n", j.args);
        printf("run: %d\n", j.run_status);
        printf("Client--------------------\n");
        struct job j2 = client_jobs[i];
        printf("order: %d\n", j2.job_order);
        printf("args:  %s\n", j2.args);
        printf("run: %d\n", j2.run_status);
    }
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
    copy_arrays(job_array, client_jobs);

    printf("======================================================\n");
    printf("=====================COMPLETED PROCESS===================\n");
    printf("======================================================\n");

    for (int i = 0; i < 20; i++) 
    {
        printf("Process-------------------\n");
        struct job j = client_jobs[i];
        printf("order: %d\n", j.job_order);
        printf("args:  %s\n", j.args);
        printf("run: %d\n", j.run_status);
        printf("Client--------------------\n");
        struct job j2 = client_jobs[i];
        printf("order: %d\n", j2.job_order);
        printf("args:  %s\n", j2.args);
        printf("run: %d\n", j2.run_status);
    }
    
}