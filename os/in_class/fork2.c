#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main() 
{
    fork();
    fork();
    printf("hello from %d; child of %d\n", getpid(), getppid());
    exit(0);
}
