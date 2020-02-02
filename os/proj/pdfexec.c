#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
/* pipe dup fork and exec Example */
/* runs as: ./pdfexec date wc  */
/* Runs the shell equivalent of date | wc */
int main(int argc, char *argv[])
{
    int pipefd[2], status, done=0;
    pid_t cpid;

    pipe(pipefd);

    cpid = fork(); 
    if (cpid == 0) {    /* left child (for date) */
	close(pipefd[0]); /* Close unused read end */
	dup2(pipefd[1],STDOUT_FILENO); /* Make output go to pipe */
	execlp(argv[1], argv[1], (char *) NULL);
    }
    cpid = fork(); 
    if (cpid == 0) {  /* right child (for wc */
	close(pipefd[1]);          /* Close unused write end */
	dup2(pipefd[0],STDIN_FILENO); /* Get input from pipe */
	execlp(argv[2], argv[2], (char *) NULL);
    }
    close(pipefd[0]); /* close pipes so EOF can work */
    close(pipefd[1]); /* This is a subtle but important step. The second child
                         will not receive a EOF to trigger it to terminate while
                         at least one other process (the parent)  has the write 
			 end open */
    
    /* Parent reaps children exits */
    waitpid(-1,&status, 0);
    waitpid(-1,&status, 0); 
}
