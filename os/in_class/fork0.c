#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main() 
{
    pid_t pid;
    int x = 1;

    pid = fork(); 
    if (pid == 0) {  /* Child */
	printf("child : x=%d\n", ++x); 
	exit(0);
    }
    /* Parent */
    printf("parent: x=%d\n", --x); 
    exit(0);
}

