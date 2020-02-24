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

#define MAX_JOBS 20

char *inString;
#pragma endregion

#pragma region FUNCTION DECLARATIONS
void sendCommandToServer(char* command);
#pragma endregion

void sendCommandToServer(char* command)
{
    
}

int main()
{
    while(inString = readline("# ")){

    }	
    return 0;
}