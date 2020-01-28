#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main() 
{
    fork();
    printf("hello\n");
    exit(0);
}
