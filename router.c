// NOTE: THIS FILE HANDLES ROUTING OF HTTP REQUESTS.
//       01) Defines routes for GET and POST requests.
//       02) Calls appropriate functions to handle requests.
//       03) Implements business logic for user registration, login, and profile management.
// NOTE: each printf statment written here is actully being sent over directly to the clientSocketDescritor becuse right before we called the void route() function we used dup2 to redirected the child process STDOUT to the clientSocketDescritor. 
//       hence there is no need to use send() API function here. 
 
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
        printf("HTTP/1.1 200 OK\r\n\r\n"); // write to the STDO which is redirected to the clientSocketDescritor a standard HTTP 200 OK.
        printf("Hello! You are using %s", request_header("User-Agent"));  // write to the STDO which is redirected to the clientSocketDescritor the request header parsed in http_protocol.c, once we return the from void route() to the httpd.c we make call fflush(stdout)Make sure any buffered output is sent to the clientSocketDescritor.  
        // load the lion image.
    }

    ROUTE_GET("/pass")
    {
        printf("HTTP/1.1 200 OK\r\n\r\n"); // write to the STDO which is redirected to the clientSocketDescritor a standard HTTP 200 OK - once we return the from void route() to the httpd.c we make call fflush(stdout)Make sure any buffered output is sent to the clientSocketDescritor.  
        printf("Hello! you got here %s , %s %s",method,uri,qs); // as explained above, and in addition sending the method, uri, qs that was analyzed and parsed via the analayzed_http in the http_protocol.c file.
    }

    ROUTE_POST("/")
    {
        printf("HTTP/1.1 200 OK\r\n\r\n"); // same as explained above in the previous routes.
        printf("Wow, seems that you POSTed %d bytes. \r\n", payload_size); // as explained above, and in addition it sends the payload size that was attached to the reqeust, which was declared as extern int in httpd.h and parsed and assign a value by the analayzed_http defined in the http_protocol.c file..
        printf("Fetch the data using `payload` variable."); // as explained above. 
    }

    ROUTE_POST("/register")
    {
        /* implement registeration:
            01) writing to the password.txt file the email and password the user has entered in the form.
        */
    }

    ROUTE_POST("/login")
    {
        /* implement login:
            01) read the password.txt file and search for a matching password.
        */
    }

    ROUTE_POST("/enterData")
    {
        /* 
            write to the files/data.txt the input the user has entered in the form. 
        */
    }

    ROUTE_POST("/editData")
    {
        /* 
            write to the files/data.txt the input the user has edit in the form. 
        */
    }

  
    ROUTE_END()
}
