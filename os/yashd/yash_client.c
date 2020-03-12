
#include <stdio.h>
/* socket(), bind(), recv, send */
#include <sys/types.h>
#include <sys/socket.h> /* sockaddr_in */
#include <netinet/in.h> /* inet_addr() */
#include <arpa/inet.h>
#include <netdb.h> /* struct hostent */
#include <string.h> /* memset() */
#include <unistd.h> /* close() */
#include <stdlib.h> /* exit() */
#include <signal.h>
#include <readline/readline.h>

#define MAXHOSTNAME 80
#define BUFSIZE 2000

char buf[BUFSIZE];
char rbuf[BUFSIZE];
void GetUserInput();
void cleanup(char *buf);

int rc, cc;
int   sd;
int childpid;
char mod_buf[2000] = {0};
char * inString;

static void sig_ignore(int signo) 
{
}

static void sig_int(int signo) 
{
    if (send(sd, "CTL C", sizeof("CTL C"), 0) <0 )
            perror("sending stream message");
    // printf("sent CTL C\n");
    // printf("in buffer: -----------------------\n%s\n------------------------", buf);
    
    // fflush(stdin);
    // fflush(stdout);
    // cleanup(mod_buf);
    // cleanup(buf);

    // int pid = fork();
    // if (pid == 0) {
	    
    //     char *args[]={"clear",NULL}; 
    //     execvp(args[0],args); 
    // }
    
}
static void sig_tstp(int signo) 
{
    if (send(sd, "CTL Z", sizeof("CTL Z"), 0) <0 )
            perror("sending stream message");
    // printf("sent CTL Z\n");
    //printf("buf: %s\n", buf);
    cleanup(mod_buf);
    cleanup(buf);
}

int main(int argc, char **argv ) {
    struct sockaddr_in server;
    struct sockaddr_in client;
    struct hostent *hp, *gethostbyname();
    struct sockaddr_in from;
    struct sockaddr_in addr;
    int fromlen;
    int length;
    char ThisHost[80];
    
    #pragma region CONNECT
    /* get TCPClient Host information, NAME and INET ADDRESS */
    
    gethostname(ThisHost, MAXHOSTNAME);
    /* OR strcpy(ThisHost,"localhost"); */
    
    // printf("----TCP/Client running at host NAME: %s\n", ThisHost);
    if  ( (hp = gethostbyname(ThisHost)) == NULL ) {
	fprintf(stderr, "Can't find host %s\n", argv[1]);
	exit(-1);
    }
    bcopy ( hp->h_addr, &(server.sin_addr), hp->h_length);
    // printf("    (TCP/Client INET ADDRESS is: %s )\n", inet_ntoa(server.sin_addr));
    
    /** get TCPServer-ex2 Host information, NAME and INET ADDRESS */
    
    if  ( (hp = gethostbyname(argv[1])) == NULL ) {
	addr.sin_addr.s_addr = inet_addr(argv[1]);
	if ((hp = gethostbyaddr((char *) &addr.sin_addr.s_addr,
				sizeof(addr.sin_addr.s_addr),AF_INET)) == NULL) {
	    fprintf(stderr, "Can't find host %s\n", argv[1]);
	    exit(-1);
	}
    }
    // printf("----TCP/Server running at host NAME: %s\n", hp->h_name);
    bcopy ( hp->h_addr, &(server.sin_addr), hp->h_length);
    // printf("    (TCP/Server INET ADDRESS is: %s )\n", inet_ntoa(server.sin_addr));
    
    /* Construct name of socket to send to. */
    server.sin_family = AF_INET; 
    /* OR server.sin_family = hp->h_addrtype; */
    
    server.sin_port = htons(3826);
    
    /*   Create socket on which to send  and receive */
    
    sd = socket (AF_INET,SOCK_STREAM,0); 
    /* sd = socket (hp->h_addrtype,SOCK_STREAM,0); */
    
    if (sd<0) {
	perror("opening stream socket");
	exit(-1);
    }

    /** Connect to TCPServer-ex2 */
    if ( connect(sd, (struct sockaddr *) &server, sizeof(server)) < 0 ) {
	close(sd);
	perror("connecting stream socket");
	exit(0);
    }
    fromlen = sizeof(from);
    if (getpeername(sd,(struct sockaddr *)&from,&fromlen)<0){
	perror("could't get peername\n");
	exit(1);
    }
    #pragma endregion
    
    // printf("Connected to TCPServer1: ");
    printf("%s:%d\n", inet_ntoa(from.sin_addr),
	   ntohs(from.sin_port));
    if ((hp = gethostbyaddr((char *) &from.sin_addr.s_addr,
			    sizeof(from.sin_addr.s_addr),AF_INET)) == NULL)
	fprintf(stderr, "Can't find host %s\n", inet_ntoa(from.sin_addr));
    else
	// printf("(Name is : %s)\n", hp->h_name);

    
    // if( (rc=recv(sd, rbuf, sizeof(buf), 0)) < 0){
    //         perror("receiving stream  message");
    //         exit(-1);
	//     }
	//     if (rc > 0){
	//         rbuf[rc]='\0';
	//         printf("%s", rbuf);
	//     }else {
	//         printf("Disconnected..\n");
	//         close (sd);
	//         exit(0);
	//     }

    if (signal(SIGINT, sig_int) == SIG_ERR)
        printf("signal(SIGINT) error");
    if (signal(SIGTSTP, sig_tstp) == SIG_ERR)
        printf("signal(SIGTSTP) error");
        
    childpid = fork();
    if (childpid == 0) {
	    GetUserInput();
    }
    else
    /** get data from USER, send it SERVER,
      receive it from SERVER, display it back to USER  */
    for(;;) 
    {

        fflush(stdout);
        cleanup(rbuf);
        if( (rc=recv(sd, rbuf, sizeof(buf), 0)) < 0){
            perror("receiving stream  message");
            exit(-1);
	    }
	    if (rc > 0){
            rbuf[rc]='\0';
            // printf("rc is : %d\n", rc);
            printf("%s", rbuf);
            rc = 0;
            fflush(stdout);
            //send(sd, "", 0, 0);
	    } else {
	        close (sd);
	        exit(0);
	    }
	
    }
}

void cleanup(char *buf)
{
    int i;
    for(i=0; i<BUFSIZE; i++) buf[i]='\0';
}

void GetUserInput()
{
     while(inString = readline(""))
     {
    // for (;;)
    // {
        // printf("\n");
        // printf("%d\n", 1);
        cleanup(buf);
        cleanup(mod_buf);

        // rc=read(0,buf, sizeof(buf));
        for (int i = 0; i < BUFSIZE; i++)
        {
            if (inString[i] == '\0')
            {
                rc = i + 1;
                break;
            }
        }
        // printf("rc: %d\n", rc);

        // printf("%d\n", 2);


        strcat(mod_buf, "CMD ");
        // strcat(mod_buf, buf);
        strcat(mod_buf, inString);

        // printf("Sending: %s\n", mod_buf);

        if (send(sd, mod_buf, rc + 4, 0) <0 )
            perror("sending stream message");
        
        cleanup(mod_buf);
        inString = "";
    }
    close(sd);
    kill(getppid(), 9);
    exit (0);
}
