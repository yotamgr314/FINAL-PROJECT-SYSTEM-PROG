// NOTE: THIS FILE HANDLES ROUTING OF HTTP REQUESTS.
//       01) Defines routes for GET and POST requests.
//       02) Calls appropriate functions to handle requests.
//       03) Implements business logic for user registration, login, and profile management.

#include "httpd.h"
#include <unistd.h>
#include <stdio.h>
#include<stdio.h> 
#include<unistd.h> 
#include<fcntl.h> 
#include<stdlib.h> 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#define BUF_SIZE 1024

void route()
{
    ROUTE_START()

    ROUTE_GET("/") // MACRO for GET http request, defined in httpd.h
    {
        printf("HTTP/1.1 200 OK\r\n\r\n");
        printf("Hello! You are using %s", request_header("User-Agent"));
    }

    ROUTE_GET("/pass")
    {
        printf("HTTP/1.1 200 OK\r\n\r\n");
        printf("Hello! you got here %s , %s %s",method,uri,qs);
    }

    ROUTE_POST("/")
    {
        printf("HTTP/1.1 200 OK\r\n\r\n");
        printf("Wow, seems that you POSTed %d bytes. \r\n", payload_size);
        printf("Fetch the data using `payload` variable.");
    }
  
    ROUTE_END()
}
