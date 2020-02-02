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

int processSingleCommand(char** args, int argc, int input_index, int output_index, int error_index) {

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
    return 0;
}

int processPipeCommands(char* leftHand, char* rightHand) {
    return 0;
}

void processOutputCommand(char* commandString, int output_index, int end_index) {

    int output_length = end_index - output_index;

    char command_string[output_index + 1];

    int b = 0;

    while (b < output_index - 1) 
    {
        command_string[b] = commandString[b];
        b++;
    }
    command_string[b] = '\0';

    char output_string[output_length + 1];

    int a = 0;
    int c = 1;
    while (c < output_length) 
    {
        if (commandString[output_index + c] != ' ') 
        {
            output_string[a] = commandString[output_index+c];
            a++;
        }
        c++;
    }
   output_string[c] = '\0';

    pid_t cpid;
    int ofd;
    cpid = fork(); 

    if (cpid == 0) { 
        ofd = creat(output_string, 0644);
        execlp(command_string, command_string, (char *)NULL);
    }

    wait((int *)NULL);
}

void processInputCommand(char* commandString, int input_index, int end_index) {
    int input_length = end_index - input_index;

    char command_string[input_length + 1];

    int b = 0;

    while (b < input_index - 1) 
    {
        command_string[b] = commandString[b];
        b++;
    }
    command_string[b] = '\0';

    char input_string[input_length + 1];

    int a = 0;
    int c = 1;
    while (c < input_length) 
    {
        if (commandString[input_index + c] != ' ') 
        {
            input_string[a] = commandString[input_index+c];
            a++;
        }
        c++;
    }
    input_string[c] = '\0';

    pid_t cpid;
    int ofd;
    cpid = fork(); 

    if (cpid == 0) { 
        ofd = open(input_string, 0644);
        dup2(ofd,0);
        execlp(command_string, command_string, (char *)NULL);
    }

    wait((int *)NULL);
}

int main(){
    char *inString;

    while(inString = readline("$ ")){
        int input_length = strlen(inString);
        int pipe_index = -1;
        int input_index = -1;
        int output_index = -1;
        int error_index = -1;

        char argv[100][35]; 
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

        char **args = malloc((ctr * sizeof(char *)) + 1);	

        for (int i = 0; i < ctr; i++) {
            args[i] = argv[i];
        }

        if (pipe_index != -1) {
            //process two separate commands
        }
        else {
            processSingleCommand(args, ctr, input_index, output_index, error_index);
        }
    }	

    return 0;
}