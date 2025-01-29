// NOTE: THIS FILE IMPLEMENTS HTTP PROTOCOL THAT WILL BE USED OVER THE TCP SOCKET PROTOCOL.
//       01) Parses HTTP requests (method, URI, headers, payload).
//       02) Extracts query parameters and request headers.
//       03) Handles HTTP request processing.
// NOTE: i do not think we need to know how to implement the parsing of an HTTP request. it just given to us so we can use HTTP protocol over the TCP socket implemenation.
// NOTE: the idea of the job - whats left to us (since are given in this template the whole TCP socket server intialization and HTTP protocol itnilization) is just to implement the controllers in the router.c - aka:
// 01) the way the each route in the server reponse to a new request, that means the send() part of the TCP socket... but remember that each printf statement in the router.c file is like a sent() function since we redirected the STDO of each newly created child to the clientSocketDescriptor 
//     and after we return from void route() defined in router.c we just flush the STDO which sends the entier printf statments as a response to the client.
//     so this work is mainly demonstarte the send() part and not the entire funcitonality of setting up a basic TCP server.(and the way we interacte with HTTP protocol above the socket API).
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct { char *name, *value; } header_t;
static header_t reqhdr[17] = { {"\0", "\0"} };

char    *method,    // "GET" or "POST"
        *uri,       // "/index.html" things before '?'
        *qs,        // "a=1&b=2"     things after  '?'
        *prot;      // "HTTP/1.1"

char    *payload;     // for POST
int      payload_size;



// get request header
char *request_header(const char* name)
{
    header_t *h = reqhdr;
    while(h->name) {
        if (strcmp(h->name, name) == 0) return h->value;
        h++;
    }
    return NULL;
}

void analyze_http(char* buf ,int rcvd){

	buf[rcvd] = '\0';
	method = strtok(buf,  " \t\r\n");
        uri    = strtok(NULL, " \t");
        prot   = strtok(NULL, " \t\r\n"); 

        fprintf(stderr, "\x1b[32m + [%s] %s\x1b[0m\n", method, uri);
        
        if (qs = strchr(uri, '?'))
        {
            *qs++ = '\0'; //split URI
        } else {
            qs = uri - 1; //use an empty string
        }

        header_t *h = reqhdr;
        char *t, *t2;

        while(h < reqhdr+16) {

            char *k,*v,*t;
            k = strtok(NULL, "\r\n: \t"); if (!k) break;
            v = strtok(NULL, "\r\n");     while(*v && *v==' ') v++;
            h->name  = k;
            h->value = v;
            h++;
            fprintf(stderr, "[H] %s: %s\n", k, v);
            t = v + 1 + strlen(v);
            if (t[1] == '\r' && t[2] == '\n')
		break;
        }

       
        t2 = request_header("Content-Length"); // and the related header if there is  
        payload_size = t2 ? atol(t2) : (rcvd-(t-buf));


	payload = buf+ rcvd-payload_size +1;
       if (payload_size <100)
            fprintf(stderr, "[H] %d %s:\n", payload_size  ,payload );

}