#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <fcntl.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h>
#include <sys/wait.h>

#define	MAXLINE	4096

void processSingleCommand(char** args, int argc, int input_index, int output_index, int error_index) {

    args[argc] = NULL;

    int cpid;

    cpid = fork();

    if (cpid == 0) {
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
    }
    else {
        wait((int *)NULL);
    }
}

void processPipeCommand(char** init_args, int argc, int pipe_index) {

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
        printf("args_left[%d]: %s\n", i, args_left[i]);
    }
    args_left[pipe_index] = NULL;

    for (int i = 0; i < (argc - pipe_index) - 1; i++) {
        args_right[i] = init_args[pipe_index + i + 1];
        printf("args_right[%d]: %s\n", i, args_right[i]);
    }
    args_right[(argc - pipe_index) - 1] = NULL;
    
    int pipefd[2];
    pid_t child1_pid;
    pid_t child2_pid;
    char buf;
    
    if (pipe(pipefd) == -1) {
        perror("pipe had an error");
    }
    else {
        child1_pid = fork();
        if (child1_pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (child1_pid == 0) {    
            /*
            *
            * WRITE SIDE
            * 
            * */
            close(pipefd[0]); 
            dup2(pipefd[1], STDOUT_FILENO); 
            close(pipefd[1]); 
            
            execvp(args_left[0], args_left);
        } 
        else {   
            /*
            *
            * READ SIDE
            * 
            * */        

            child2_pid = fork();

            if (child2_pid == -1) {
                printf("Child 2 had a problem\n");
            }
            else if (child2_pid == 0) {

                close(pipefd[1]); 
                dup2(pipefd[0], STDIN_FILENO); 
                close(pipefd[0]);  
                
                execvp(args_right[0], args_right);
            }
            else {
                wait((int *)NULL);
            }
        }
    }
}

int main(){
    char *inString;

    while(inString = readline("$ ")){
        int input_length = strlen(inString);
        int pipe_index = -1;
        int input_index = -1;
        int output_index = -1;
        int error_index = -1;

        char argv[100][50]; 
        int j,ctr;
        
        j=0; ctr=0;
        for(int i=0;i<=(strlen(inString));i++) {

            if(inString[i] == '|') {
                pipe_index = ctr;
            }
            if(inString[i] == '<') {
                input_index = ctr;
            }
            if(inString[i] == '>') {
                if (inString[i-1] == '2') {
                    error_index = ctr;
                }
                else  {
                    output_index = ctr;
                } 
            }
            
            // if space or NULL found, assign NULL into newString[ctr]
            if(inString[i]==' '||inString[i]=='\0') {
                argv[ctr][j]='\0';
                ctr++;  //for next word
                j=0;    //for next word, init index to 0
            }
            else {
                argv[ctr][j]=inString[i];
                j++;
            }
        }

        char **args = malloc((ctr  + 1)* sizeof(char *));	

        for (int i = 0; i < ctr; i++) {
            args[i] = argv[i];
        }

        if (pipe_index != -1) {
            processPipeCommand(args, ctr, pipe_index);
        }
        else {
            processSingleCommand(args, ctr, input_index, output_index, error_index);
        }
    }	

    return 0;
}