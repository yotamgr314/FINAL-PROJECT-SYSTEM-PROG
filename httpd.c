// NOTE: THIS FILE CONTAINS TCP SOCKET IMPLEMENTATION.
//       01) creates TCP socket with socket()
//       02) binds the socket to the port.
//       03) listens to incoming requests via listen()
//       04) handling handshake via the accept() function.

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
static void startServer(const char *); // starts the server on the specified port.
static void respond(int); // handles client's requests. 



// Main server function loop: listen and handles clients connections.
void serve_forever(const char *PORT)
{
    struct sockaddr_in clientaddr; // Structure to obtain the client address information.
    socklen_t addrlen; // the address length.
    char c; // Unused variable, potentially for deubbgin or future use.
    
    int slot=0; // variable to track the next available slot for a client
    

    printf("Server started %shttp://127.0.0.1:%s%s\n","\033[92m",PORT,"\033[0m"); // print a message to indicate the server has started. 

    // Setting all elements to -1 to signify that there is no connected clients.
    int i;
    for (i=0; i<CONNMAX; i++)
    {
        clients[i]=-1;
    }

    startServer(PORT); // Start the server on the specified port
    
    // Ignore SIGCHLD to avoid zombie threads
    signal(SIGCHLD,SIG_IGN);

    // thats the inifnite while loop sheba ha socket ha mesharet continutes to accept newly connected. we are here only after the startServer func was called, which has inner call to listen() func API. so at this point we are listening la socket ha meshaeret.
    while (1) 
    {
        addrlen = sizeof(clientaddr);
        clients[slot] = accept (listenfd, (struct sockaddr *) &clientaddr, &addrlen);// accept returns a socket descriptor which assigned to the clients array bamakom a slot, listenfd hu ha file descriptor shel a socket a mehsaret.

        if (clients[slot]<0)
        {
            perror("accept() error"); // if the acccept failed we send an error
        }
        else // only if accept did not failed, aka returned value > 0 it means it returned the file descriotpor of the newly connected client. 
        {
            if ( fork()==0 ) // creates a child to handle the newly accepted connection.
            {
                respond(slot); // this function is incharge of reciving the request of the newly accept connection, etc - recv(), and sendto(). we pass the respond() the index of the newly connected client inside the clients array. that index comtains its socket dicriptor.
                exit(0);
            }
        }

        while (clients[slot]!=-1) slot = (slot+1)%CONNMAX; //  now clients[slut] points to the next avaliable slut in the clients array and we conitnute to the next iteration which calls accept() 
    }
}

//start server
void startServer(const char *port)
{
    struct addrinfo hints; // A struct used to specify criteria for selecting socket address structures, such as protocol family (IPv4), socket type (TCP), and flags (e.g., passive mode for binding).
    struct addrinfo *res; //A pointer to a linked list of addrinfo structures returned by `getaddrinfo`. it Stores the result of getaddrinfo, which provides one or more potential socket address structures matching the criteria defined in hints.
    struct addrinfo *p; // A pointer used to iterate through the linked list in `res` to test each socket address until a valid one is found and successfully bound.

    // הגדרת כתובת השרת
    memset (&hints, 0, sizeof(hints)); // Zero out the hints structure
    hints.ai_family = AF_INET; //  Use IPv4
    hints.ai_socktype = SOCK_STREAM; // ---> INDICATES ITS A TCP SOCKET ! 
    hints.ai_flags = AI_PASSIVE; // Bind to all available interfaces.
    
    if (getaddrinfo( NULL, port, &hints, &res) != 0) // func defined in #include <netdb.h> which stores in res a pointer to linked list of possible addresses that match the fields defined  in hints. etc - an address that match family - AF_INET --> supports IPV4 addressed only. hints.ai_socktype = SOCK_STREAM --> address which supports TCP sockets.  
    {
        perror ("getaddrinfo() error");
        exit(1);
    }
    // socket and bind
    for (p = res; p!=NULL; p=p->ai_next) // we iterate through all the possible addresses  structs that getaddrinfo() func as returned, and try to bind our socket to it. upon the first the bind of the socket with an address succseed we break out of the for loop.
    {
        int option = 1;
        listenfd = socket (p->ai_family, p->ai_socktype, 0); // creating a socket with the parameters defined in the addresses struct that getaddrinfo returned. (each struct in that LL contains the fields defined in hints as well)
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)); // Set socket option SO_REUSEADDR to allow reusing the address immediately after closing
        if (listenfd == -1) continue;
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0) // Attempt to bind the socket to the current address struct in the list
        {
            break; // if binds sucessed exit the loop sicne we have a valid socket bound to a struct adress. 
        } 
    }

// NOTE: THE EQUIVALENT TO if(getaddrinfo) in summer-bet-q5-tcp-version.
/* sockfd = socket(AF_INET, SOCK_STREAM, 0);
if (sockfd < 0) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
}

memset(&servaddr, 0, sizeof(servaddr));
servaddr.sin_family = AF_INET;
servaddr.sin_addr.s_addr = INADDR_ANY;
servaddr.sin_port = htons(PORT);

if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    perror("Bind failed");
    close(sockfd);
    exit(EXIT_FAILURE);
}
 */
    if (p==NULL)
    {
        perror ("socket() or bind()");
        exit(1);
    }

    freeaddrinfo(res);
    
    if ( listen (listenfd, 1000000) != 0 ) // listens la socket ha mesharet, can handle up to 1000000 waiting requests to connect. 
    {
        perror("listen() error");
        exit(1);
    }
}


static char *buf;

//client connection
void respond(int n) // n is the socket descriptor of the newly connects client.
{
    	int rcvd, fd, bytes_read;
    	char *ptr;

    	buf = malloc(65535); // allocated enough space for the buffer to contain the newly client request content.

    	rcvd=recv(clients[n], buf, 65535, 0); // we use the recv() to recivie the request content from the clientSocketDescriptor which is in the - clients[n]. and saving it into buffer.
    
    	if (rcvd<0)    // receive error
        	fprintf(stderr,("recv() error\n"));
    	else if (rcvd==0)    // receive socket closed
        	fprintf(stderr,"Client disconnected unexpectedly.\n");
    	else  //message received
        	 analyze_http(buf ,rcvd); // if we are here it means we mannage to recivie the content of the newly connected client from its clientSocketDescritor. and we just need to parse its HTML content by the HTML protocol implement in http_protocol.c 

	 // Redirect standard output (stdout) of the server child process to the clientSocketDescritor 
     // allowing all printf() calls to send data directly to the client - NOTE: this part replaces the send() part in summer-bet-q5-tcp-version.c since from now on we can just print to the STDO and it will be redirected to the clientSocketDescritor.
        clientfd = clients[n]; //clientfd - now contains the clientSocketDescritor which is in the clients[n] index.
        dup2(clientfd, STDOUT_FILENO); // makes all printf() statements output data to the client socket instead of the terminal.
        close(clientfd); // First close: We close the original `clientfd` because it has been duplicated to `STDOUT_FILENO`.
                         // This does NOT close the connection, since the duplicated descriptor (stdout) is still open.

        // Call the router function to switch case and match the apropiate route for the current request, each route will handle the needed request and send a response directly via printf statment which are redirected to the clientSocketDescritor
        route();

        // Tidy up - Flush any remaining output to ensure it's sent before closing
        fflush(stdout); // Make sure any buffered output is sent to the client 
        shutdown(STDOUT_FILENO, SHUT_WR); // Shutdown writing on `stdout`, preventing further writes
        close(STDOUT_FILENO); // Close `stdout` as we no longer need it

        // Closing the client socket connection
    	shutdown(clientfd, SHUT_RDWR);//All further send and recieve operations are DISABLED...
    	close(clientfd);// Second close: Now we fully close the socket connection.
                        // The first close only removed the extra descriptor, but this actually terminates the connection.
    	clients[n]=-1; // Mark the client slot as available again
}
