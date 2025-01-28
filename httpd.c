#include "httpd.h" // for a custom HTTP server header (contains declarations, macros, etc..).
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h> // For system calls (fork, dup2, close, pipe, etc..).
#include <sys/types.h> // Defines data types used in system calls (pid_t, size_t, etc...)
#include <sys/stat.h> // Provides functions and macros for dealing with file attributes and status (e.g., stat, chmod)
#include <sys/socket.h> // Socket API (e.g., socket creation, bind, listen, accept).
#include <arpa/inet.h> // Functions for IP address handling (e.g., inet_ntoa, htons).
#include <netdb.h>  // Network database functions like getaddrinfo for resolving hostnames and service names
#include <fcntl.h> // Provides file control options (e.g., non-blocking file descriptors, O_RDONLY, O_WRONLY).
#include <signal.h> // Signal handling for processes, SIGCHILD, etc..

#define CONNMAX 1000 // Maximum number of simultaneous client connections

// Global variables for server and client management
static int listenfd; // A global integer variable to store the server's listening socket descriptor.
static int clients[CONNMAX]; // A global array of integers to store the socket descriptors of connected clients.
static int clientfd; // A global integer variable to store the current client socket descriptor being handled.

// Function prototypes for server initialization and error handling
static void error(char *);
static void startServer(const char *);
static void respond(int);



// the server's main while forever serving loop.
void serve_forever(const char *PORT)
{
    struct sockaddr_in clientaddr; // Structure to obtain the client address information.
    socklen_t addrlen; // the address length.
    char c; 
    
    int slot=0;
    
    printf(
            "Server started %shttp://127.0.0.1:%s%s\n",
            "\033[92m",PORT,"\033[0m"
            );

    // Setting all elements to -1: signifies there is no client connected
    int i;
    for (i=0; i<CONNMAX; i++)
        clients[i]=-1;
    startServer(PORT);
    
    // Ignore SIGCHLD to avoid zombie threads
    signal(SIGCHLD,SIG_IGN);

    // ACCEPT connections
    while (1)
    {
        addrlen = sizeof(clientaddr);
        clients[slot] = accept (listenfd, (struct sockaddr *) &clientaddr, &addrlen);

        if (clients[slot]<0)
        {
            perror("accept() error");
        }
        else
        {
            if ( fork()==0 )
            {
                respond(slot);
                exit(0);
            }
        }

        while (clients[slot]!=-1) slot = (slot+1)%CONNMAX;
    }
}

//start server
void startServer(const char *port)
{
    struct addrinfo hints, *res, *p;

    // getaddrinfo for host
    memset (&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo( NULL, port, &hints, &res) != 0)
    {
        perror ("getaddrinfo() error");
        exit(1);
    }
    // socket and bind
    for (p = res; p!=NULL; p=p->ai_next)
    {
        int option = 1;
        listenfd = socket (p->ai_family, p->ai_socktype, 0);
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
        if (listenfd == -1) continue;
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) break;
    }
    if (p==NULL)
    {
        perror ("socket() or bind()");
        exit(1);
    }

    freeaddrinfo(res);

    // listen for incoming connections
    if ( listen (listenfd, 1000000) != 0 )
    {
        perror("listen() error");
        exit(1);
    }
}


static char *buf;

//client connection
void respond(int n)
{

    	int rcvd, fd, bytes_read;
    	char *ptr;

    	buf = malloc(65535);
    	rcvd=recv(clients[n], buf, 65535, 0);
    
    	if (rcvd<0)    // receive error
        	fprintf(stderr,("recv() error\n"));
    	else if (rcvd==0)    // receive socket closed
        	fprintf(stderr,"Client disconnected unexpectedly.\n");
    	else  //message received
        	 analyze_http(buf ,rcvd);

	 // bind clientfd to stdout, making it easier to write
        clientfd = clients[n];
        dup2(clientfd, STDOUT_FILENO);
        close(clientfd);

        // call router
        route();

      // tidy up
        fflush(stdout);
        shutdown(STDOUT_FILENO, SHUT_WR);
        close(STDOUT_FILENO);

    	//Closing SOCKET
    	shutdown(clientfd, SHUT_RDWR);         //All further send and recieve operations are DISABLED...
    	close(clientfd);
    	clients[n]=-1;
}
