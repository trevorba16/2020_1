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

#define	MAXLINE	4096
#define MAX_JOBS 20

struct job
{
    int pid;
    int run_status; // 1 is running, 0 is stopped
    int job_order;
    char **args;
};

int pid_ch1, pid_ch2, ppid, status;
struct job job_array[20];


static void sig_int(int signo) 
{
//   printf("Sending signals to group:%d\n",pid_ch1);
  kill(pid_ch1,SIGINT);
}
static void sig_tstp(int signo) 
{
//   printf("Sending SIGTSTP to group:%d\n",pid_ch1); 
    kill(pid_ch1,SIGTSTP);
    struct job new_job;

    new_job.pid = pid_ch1;
    new_job.run_status = 16;
    new_job.job_order = 17;

    for (int i = 0; i < MAX_JOBS; i++) 
    {
        if job_array[i] 

    }

}

void processSingleCommand(char** args, int argc, int input_index, int output_index, int error_index, int background_index) 
{
    args[argc] = NULL;

    pid_ch1 = fork();
    if (pid_ch1 > 0) 
    {
        // Parent

        setpgid(pid_ch1, pid_ch1);

        if (signal(SIGINT, sig_int) == SIG_ERR)
            printf("signal(SIGINT) error");
        if (signal(SIGTSTP, sig_tstp) == SIG_ERR)
            printf("signal(SIGTSTP) error");

        int count = 0;
        while (count < 1) 
        {
            ppid = waitpid(-1, &status, WUNTRACED);
            // printf("waiting\n");
            
            if (ppid == -1) 
            {
                perror("waitpid");
                exit(EXIT_FAILURE);
            }

            if (WIFEXITED(status)) 
            {
                //printf("child %d exited, status=%d\n", ppid, WEXITSTATUS(status));
                count++;
            } 
            else if (WIFSIGNALED(status)) 
            {
                //printf("child %d killed by signal %d\n", ppid, WTERMSIG(status));
                count++;
            } 
            else if (WIFSTOPPED(status)) {
                //printf("%d stopped by signal %d\n", ppid,WSTOPSIG(status));
                count++;
            } else if (WIFCONTINUED(status)) {
                //printf("Continuing %d\n",ppid);
            }
        }
    }
    else if (pid_ch1 == 0) {
        // Child

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

        execvp(args[0], args);
        exit(0);
    }
}

void processPipeCommand(char** init_args, int argc, int pipe_index, int background_index) 
{

    char **args_left = malloc((argc) * sizeof(char *));
    char **args_right = malloc((argc) * sizeof(char *));

    int left_input = -1;
    int left_output = -1;
    int left_error = -1;

    int right_input = -1;
    int right_output = -1;
    int right_error = -1;

    for (int i = 0; i < pipe_index; i++) {
        args_left[i] = init_args[i];
        // printf("args_left[%d]: %s\n", i, args_left[i]);

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

    // printf("Output Index: %d\n", left_output);

    for (int i = 0; i < (argc - pipe_index) - 1; i++) {
        args_right[i] = init_args[pipe_index + i + 1];
        // printf("args_right[%d]: %s\n", i, args_right[i]);

          if (args_right[i][0] == '>') {
            left_output = i;
        }
        else if (args_right[i][0] == '<') {
            left_input = i;
        }
        else if (args_right[i][0] == '2' && args_right[i][1] == '>') {
            left_error = i;
        }
    }
    args_right[(argc - pipe_index) - 1] = NULL;
    
    int pipefd[2];
    char buf;
    
    if (pipe(pipefd) == -1) {
        perror("pipe had an error");
    }
    else {
        pid_ch1 = fork();
        if (pid_ch1 == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if (pid_ch1 == 0) {    
            /*
            *
            * WRITE SIDE / Child 1
            * 
            * */
            close(pipefd[0]); 
            dup2(pipefd[1], STDOUT_FILENO); 
            close(pipefd[1]); 

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
        else {   
            /*
            *
            * READ SIDE
            * 
            * */        

            pid_ch2 = fork();

            if (pid_ch2 == -1) {
                printf("Child 2 had a problem\n");
                exit(EXIT_FAILURE);
            }
            if (pid_ch2 == 0) {

                // Child 2
                sleep(1);
                setpgid(0, pid_ch1);

                close(pipefd[1]); 
                dup2(pipefd[0], STDIN_FILENO); 
                close(pipefd[0]);  

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

                execvp(args_right[0], args_right);
                exit(0);
            }
            else {
                // Parent
                if (signal(SIGINT, sig_int) == SIG_ERR)
                    printf("signal(SIGINT) error");
                if (signal(SIGTSTP, sig_tstp) == SIG_ERR)
                    printf("signal(SIGTSTP) error");

                int count = 0;
                while (count < 2) 
                {
                    ppid = waitpid(-1, &status, WUNTRACED);
                    // printf("waiting\n");
                    
                    if (ppid == -1) 
                    {
                        perror("waitpid");
                        exit(EXIT_FAILURE);
                    }

                    if (WIFEXITED(status)) 
                    {
                        // printf("child %d exited, status=%d\n", ppid, WEXITSTATUS(status));
                        count++;
                    } 
                    else if (WIFSIGNALED(status)) 
                    {
                        // printf("child %d killed by signal %d\n", ppid, WTERMSIG(status));
                        count++;
                    } 
                    else if (WIFSTOPPED(status)) {
                        // printf("%d stopped by signal %d\n", ppid,WSTOPSIG(status));
                        count++;
                    } else if (WIFCONTINUED(status)) {
                        // printf("Continuing %d\n",ppid);
                    }
                }
            }
        }
    }
}

int getMostRecentBackground()
{
    printf("Starting getMostRecentBackground()\n");
    int max_process = -1;
    int max_job_order = -1;

    
    for (e = list_begin (&job_list); e != list_end (&job_list);
	  e = list_next (e))
     {
        struct job *j =
        printf("Made job\n");
        printf("run_status: %d\n", j->run_status);
        printf("jobOrder: %d\n", j->job_order);
        printf("pid: %d\n", j->pid);
        if (j->run_status == 0 && j->job_order > max_job_order)
        {
            
            max_job_order = j->job_order;

            max_process = j->pid;
        }
    }
    return max_process;
}

void startJob(int jobOrder) {

}

void processForegroundCommand() 
{
    int max_pid = getMostRecentBackground();
    //printf("Waiting for: %d\n", max_pid);
    //int fg_pid = waitpid(max_pid, &status, WUNTRACED);

    
    // if (WIFEXITED(status)) 
    // {
        
    // } 
    // else if (WIFSIGNALED(status)) 
    // {
        
    // } 
    // else if (WIFSTOPPED(status)) 
    // {
        printf("Continuing: %d\n", max_pid);
        kill(max_pid, SIGCONT);
        printf("Continued: %d\n", max_pid);
    // } 
    // else if (WIFCONTINUED(status)) 
    // {

    // }
}

int main()
{
    char *inString;

    while(inString = readline("$ ")){

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
                background_index = i;
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

        if (foreground == 1) 
        {
            processForegroundCommand();
        }
        else if (background == 1)
        {
            //processBackgroundCommand();
        }
        else if (jobs == 1)
        {
            //processJobsCommand();
        }
        else 
        {
            if (pipe_index != -1) 
            {
                processPipeCommand(args, ctr, pipe_index, background_index);
            }
            else 
            {
                processSingleCommand(args, ctr, input_index, output_index, error_index, background_index);
            }
        }
    }	
    return 0;
}