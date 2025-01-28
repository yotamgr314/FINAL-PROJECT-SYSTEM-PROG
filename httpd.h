#ifndef _HTTPD_H___
#define _HTTPD_H___

#include <string.h>
#include <stdio.h>

//Server control functions

void serve_forever(const char *PORT);

//Client request
extern char    *method,    // "GET" or "POST"
        *uri,       // "/index.html" things before '?'
        *qs,        // "a=1&b=2"     things after  '?'
        *prot;      // "HTTP/1.1"

extern char    *payload;     // for POST
extern int      payload_size;

char *request_header(const char* name);
void analyze_http(char* buf,int rcvd);

//user shall implement this function
void route();


// Macros that define a simple routing mechanism in C.

// Marks the start of the routing chain with an always-false `if (0)`
#define ROUTE_START()       if (0) { // is simply a trick to begin a chain of } else if (...) { … }.

// Defines an else-if condition that checks if `method` and `uri` match the arguments
#define ROUTE(METHOD,URI)   } else if (strcmp(URI, uri) == 0 && strcmp(METHOD, method) == 0) {

// a GET macro shortcut that for a nother macro shortcut to a ROUTE func. calls the ROUTE macro which is a macro to a function as well, it calls ROUTE func with a "GET" as first parameter, and URI that the client sent as second parameter.("GET", URI).
#define ROUTE_GET(URI)      ROUTE("GET", URI)

// a GET macro shortcut that for a nother macro shortcut to a ROUTE func. calls the ROUTE macro which is a macro to a function as well, it calls ROUTE func with a "POST" as first parameter, and URI that the client sent as second parameter.("POST", URI).
#define ROUTE_POST(URI)     ROUTE("POST", URI)

// Ends the chain of else-if routes. If no routes matched, respond with 500.
#define ROUTE_END()         } else printf(\
                                "HTTP/1.1 500 Not Handled\r\n\r\n" \
                                "The server has no handler to the request.\r\n" \
                            );

#endif




