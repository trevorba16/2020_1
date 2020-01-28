#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>

char ** parseString(char * str); //parses an command into strings 
/* exec Example 2 */
int main(){
    int cpid;
    char *inString;
    char **parsedcmd;

    while(inString = readline("cmd:")){

	parsedcmd = parseString(inString);
	cpid = fork();
	if (cpid == 0){
	    execvp(parsedcmd[0],parsedcmd);
	}else{
	    wait((int *)NULL);
	}
    }
}

// parseString function omitted (read the man page for strtok)
