#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <fcntl.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h>
#include <sys/wait.h>

int processSingleCommand(char* commandString) {

    char newString[10][10]; 
    int i,j,ctr;  
    j=0; ctr=0;
    for(i=0;i<=(strlen(commandString));i++)
    {
        // if space or NULL found, assign NULL into newString[ctr]
        if(commandString[i]==' '||commandString[i]=='\0')
        {
            newString[ctr][j]='\0';
            ctr++;  //for next word
            j=0;    //for next word, init index to 0
        }
        else
        {
            newString[ctr][j]=commandString[i];
            j++;
        }
    }
    
    char *args[ctr + 1];
    
    for (int i = 0; i < ctr; i++) {
        args[i] = newString[i];
    }

    args[ctr] = NULL;

    int cpid;

    cpid = fork();

    if (cpid == 0) {
        //printf("%s\n", args[0]);
        execvp(args[0], args);//, (char *) NULL);
    }
    else{
        wait((int *)NULL);
    }

    return 0;
}

int processPipeCommands(char* leftHand, char* rightHand) {
    return 0;
}

int processOutputCommand(char* commandString, int output_index, int end_index) {

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
    printf("Output Length: %d\n", output_length);
    while (c < output_length) 
    {
        if (commandString[output_index + c] != ' ') 
        {
            printf("Index: %d", c);
            printf("   Character: %c\n", commandString[output_index + c]);
            output_string[a] = commandString[output_index+c];
            a++;
        }
        c++;
    }
   output_string[c] = '\0';

    pid_t cpid;
    int ofd;
    cpid = fork(); 

    if (cpid == 0) {  /* Child */
        printf("Command: %s\n", command_string);
        printf("Output file: %s\n", output_string);
        //printf("Running like : %s > %s\n",command_string, output_string);
        ofd = creat(output_string, 0644);
        dup2(ofd,1);   // same as: close(1); dup(ofd)
        execlp(command_string, command_string, (char *)NULL);
    }

    wait((int *)NULL);
}

int processDefaultCommand(char* commandString, int input_index, int output_index) {
    if (input_index == -1 && output_index == -1) {
        //no input or output
        processSingleCommand(commandString);
    }
    else if (input_index != -1 && output_index == -1) {
        //only input
    }
    else if (output_index != -1 && input_index == -1) {
        //only output
        processOutputCommand(commandString, output_index, strlen(commandString));
    }
}


int main(){
    char *inString;

    while(inString = readline("$ ")){
        int input_length = strlen(inString);
        int pipe_index = -1;
        int input_index = -1;
        int output_index = -1;

        // int wordCount = 0;
        for(int i = 0; i <= input_length; i++)
        {
            if(inString[i] == '|')
            {
                pipe_index = i;
            }
            else if (inString[i] == '<') {
                input_index = i;
            }
            else if (inString[i] == '>') {
                output_index = i;
            }
        }

        if (pipe_index != -1) {
            //process two separate commands
        }
        else {
            processDefaultCommand(inString, input_index, output_index);
        }
    }	

    return 0;
}

/*
#include <stdio.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h>
#include <sys/wait.h>


void splitStatement(char* statement, char* args[]) {
    char newString[10][10]; 
    int i,j,ctr;  
    j=0; ctr=0;
    for(i=0;i<=(strlen(statement));i++)
    {
        // if space or NULL found, assign NULL into newString[ctr]
        if(statement[i]==' '||statement[i]=='\0')
        {
            newString[ctr][j]='\0';
            ctr++;  //for next word
            j=0;    //for next word, init index to 0
        }
        else
        {
            newString[ctr][j]=statement[i];
            j++;
        }
    }
    
    for (int i = 0; i < ctr; i++) {
        args[i] = newString[i];
    }
}

int wordCount(char* statement) {
    int i,ctr;  
    ctr=0;
    for(i=0;i<=(strlen(statement));i++)
    {
        // if space or NULL found, assign NULL into newString[ctr]
        if(statement[i]==' '||statement[i]=='\0')
        {
            ctr++;  //for next word
        }
    }
    return ctr;
}

int processSingleCommand(char* commandString) {

    int cnt = wordCount(commandString);

    char *args[cnt + 1];
    
    splitStatement(commandString, args);

    args[cnt] = NULL;
    
    int cpid;

    cpid = fork();

    if (cpid == 0) {
        for (int i = 0; i < cnt + 1; i++) {
        printf("%s++++++++\n", args[i]);
    }
        execvp(args[0], args);//, (char *) NULL);
    }
    else{
        wait((int *)NULL);
    }

    return 0;
}

int processPipeCommands(char* leftHand, char* rightHand) {
    return 0;
}

int processDefaultCommand(char* commandString, int input_index, int output_index) {
    if (input_index == -1 && output_index == -1) {
        processSingleCommand(commandString);
    }
}


int main(){
    char *inString;

    while(inString = readline("$ ")){
        int input_length = strlen(inString);
        int pipe_index = -1;
        int input_index = -1;
        int output_index = -1;

        // int wordCount = 0;
        for(int i = 0; i <= input_length; i++)
        {
            if(inString[i] == '|')
            {
                pipe_index = i;
            }
            else if (inString[i] == '<') {
                input_index = i;
            }
            else if (inString[i] == '>') {
                output_index = i;
            }
        }

        if (pipe_index != -1) {
            //process two separate commands
        }
        else {
            processDefaultCommand(inString, input_index, output_index);
        }
    }	

    return 0;
}









// // printf("%s\n", "1");
//         char args[50][50];
//         int counter = 0;
//         int j = 0;
//         // printf("%s\n", "1.1");
//         for(int i = 0; i <= input_length; i++)
//         {
//             if(inString[i]==' ' || inString[i]=='\0')
//             {
//                 // printf("%s\n", "1.2");
//                 args[counter][j]='\0';
//                 counter++;  
//                 j=0;
//                 // printf("%s\n", "1.3");
//             }
//             else if (inString[i] == '<' || inString[i] == '|' 
//                 || inString[i] == '>' )
//             {
//                 if (j == 0) {
//                     // printf("%s\n", "1.4");
//                     args[counter][j]=inString[i];
//                     args[counter][j + 1]='\0';
//                     if (inString[i + 1] == ' ') {
//                         i++;
//                     }
//                     counter++;
//                     // printf("%s\n", "1.5");
//                 }
//                 else {
//                     // printf("%s\n", "1.6");
//                     args[counter][j]='\0';
//                     counter++;
//                     j = 0;
//                     args[counter][j]=inString[i];
//                     args[counter][j + 1]='\0';
//                     counter++;
//                     // printf("%s\n", "1.7");
//                 }
//             }
//             else
//             {
//                 // printf("%s\n", "1.8");
//                 args[counter][j] = inString[i];
//                 j++;
//                 // printf("%s\n", "1.9");
//             }
//         }
//         // printf("%s\n", "2");
//         for (int i = 0; i < 50; i++) {
//             printf("%s\n", args[i]);
//         }

*/