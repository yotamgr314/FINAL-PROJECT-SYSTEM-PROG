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

// CONTROLERS SECTION
/* ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- */
// Function to decode URL-encoded characters (e.g., `%40` → `@`), The email entered yotamkiki@gmail.com appears as yotamkiki%40gmail.com. the  @ symbol is URL-encoded as %40 (because form data is sent using application/x-www-form-urlencoded
void url_decode(char *src, char *dest) {
    char *p = src;
    char code[3] = {0};

    while (*p) {
        if (*p == '%' && p[1] && p[2]) {
            code[0] = p[1];
            code[1] = p[2];
            *dest++ = (char)strtol(code, NULL, 16); // Convert `%xx` to char
            p += 3; // Skip past `%xx`
        } else if (*p == '+') {
            *dest++ = ' '; // Convert `+` to space
            p++;
        } else {
            *dest++ = *p++; // Copy normal characters
        }
    }
    *dest = '\0'; // Null-terminate decoded string
}



// Generic Cenetrelized logic for send files method which utilizied printf due the dup2. 
void send_file(const char *file_path, const char *content_type) {
    int fd = open(file_path, O_RDONLY); // gets the index.html/css/imagefile file descritor. 
    if (fd == -1)
    {
        printf("HTTP/1.1 404 Not Found\r\n\r\n");
        printf("Error: Could not open %s.\r\n", file_path);
        fflush(stdout);
        return;
    }
    
    char buffer[BUF_SIZE];
    ssize_t bytes_read;

    printf("HTTP/1.1 200 OK\r\n");
    printf("Content-Type: %s\r\n\r\n", content_type);
    fflush(stdout);

    while ((bytes_read = read(fd, buffer, BUF_SIZE)) > 0) // read the index.html/css/image file content its all bytes in the end..
    {
        fwrite(buffer, 1, bytes_read, stdout); // writes the index.html/css/image read from the buffer into the STDOUT.
        fflush(stdout);
    }

    close(fd);
}


// writes into the password.txt the new user name and password in application/x-www-form-urlencoded format - username=yotamgr@gmail.com&password=123. returns 0 if username do not exist in the payload(body) of the request. 1 if succeed to write into password.txt.
int register_user(const char* payload)
{
    
    if (strstr(payload, "username=") == NULL) // strstr searches fro the substring username inside the payloard string.
    {
        return 0;
    }

    char decoded_payload[1024];  // Buffer for decoded data
    url_decode((char *)payload, decoded_payload); // Decode URL-encoded data

    FILE *passwordFileDescriptor = fopen("./files/password.txt", "a"); // the "a" mode means append mode, adds to the end of the file, and if the file do not exist it opens it. 
    if (!passwordFileDescriptor) 
    {
        return 0;
    }

    fprintf(passwordFileDescriptor, "%s\n", decoded_payload); // prints the payload into the password.txt file, via changing the fprintf stdout into the passwordFileDescriptor.
    fclose(passwordFileDescriptor);
    return 1;
}

// find function - returns 1 if username exists in in password.txt, and 0 if not.
int user_exists(const char* newUserName)
{
    FILE *passwordFileDescriptor = fopen("./files/password.txt", "r");
    if (!passwordFileDescriptor) {
        return 0;
    }

    char stored_payload[BUF_SIZE];
    char stored_username[100];
    while (fgets(stored_payload, sizeof(stored_payload), passwordFileDescriptor))
    {
        sscanf(stored_payload, "username=%[^&]", stored_username); // scans into stored_payload all the usernames from passwordFileDescritor and if a username matches the newUserName it returns 1, else 0.
        if (strcmp(newUserName, stored_username) == 0) 
        {
            fclose(passwordFileDescriptor);
            return 1;
        }
    }

    fclose(passwordFileDescriptor);
    return 0;
}


// Function to check if a user exists in password.txt, if he does return 1, else return 0.
int login_user(const char* payload) 
{
    FILE *passwordFileDescriptor = fopen("./files/password.txt", "r");
    if (!passwordFileDescriptor) 
    {
        return 0;  
    }

    char stored_payload[BUF_SIZE];  
    char decoded_payload[BUF_SIZE];

    // Decode only the input payload (user's inserted login email)
    url_decode((char *)payload, decoded_payload);

    // Read each line from the file
    while (fgets(stored_payload, sizeof(stored_payload), passwordFileDescriptor)) 
    {
        // Remove the newline character (`\n`) at the end of each line
        stored_payload[strcspn(stored_payload, "\n")] = '\0';

        // Compare the decoded input with the stored credentials
        if (strcmp(decoded_payload, stored_payload) == 0) 
        {
            fclose(passwordFileDescriptor);  
            return 1;  // Return 1 if the credentials match (Login Successful)
        }
    }

    fclose(passwordFileDescriptor);  
    return 0;  // Return 0 if no match is found (Login Failed)
}

/* ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- */




// ROUTES SECTION
/* ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- */
void route()
{
    ROUTE_START()
/* --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- */
    // NOTE: THESE ARE THE ROTUES FOR INDEX.HTML PAGE.
    ROUTE_GET("/") // MACRO for GET http request, defined in httpd.h
    {
        // write to the STDO which is redirected to the clientSocketDescritor a standard HTTP 200 OK.
        // write to the STDO which is redirected to the clientSocketDescritor the request header parsed in http_protocol.c, once we return the from void route() to the httpd.c we make call fflush(stdout)Make sure any buffered output is sent to the clientSocketDescritor.  
        // load the lion image.
        send_file("./files/index.html", "text/html");
    }

    ROUTE_POST("/register")
    {
        char username[100] = {0};
        sscanf(payload, "username=%[^&]", username); //, Payload is analyzed and parsed in the given http_protocl_c file . the sscanf Start scanning from where "username=" appears in the string. and Extract everything after "username=" until & is found. (becuse it expects it to be in application/x-www-form-urlencoded which means the body of the request is "username=john&password=123")
        if (user_exists(username)) 
        {
            printf("HTTP/1.1 302 Found\r\n"); // 302 reponse for automatice redirection.
            printf("Location: /?response=RegisterFailedUserAleardyExists\r\n\r\n"); // redirect the page to location: (specified location.)
            fflush(stdout);
        } else {
            if (register_user(payload)) 
            {
                printf("HTTP/1.1 302 Found\r\n"); // write to the STDO which is redirected to the clientSocketDescritor a standard HTTP 302 Found, as the route was found. - once we return the from void route() to the httpd.c we make call fflush(stdout)Make sure any buffered output is sent to the clientSocketDescritor.  
                printf("Location: /?response=RegisterSuccess\r\n\r\n"); // as explained above.
                fflush(stdout);
            } else 
            {
                printf("HTTP/1.1 302 Found\r\n");
                printf("Location: /?response=dbErrorFailedToAppendNewData\r\n\r\n");
                fflush(stdout);
            }
        }
    }

    // Route for serving the reference made by index.html to index_style.css --> once the index.html is sent to the browser the browser will make another GET request for each of the referenced files inside index.html.
    ROUTE_GET("/index_style.css")
    {
        send_file("./files/index_style.css", "text/css");
    }

    // Route for serving the reference made by index.html to index_script.js --> once the index.html is sent to the browser the browser will make another GET request for each of the referenced files inside index.html.
    ROUTE_GET("/index_script.js")
    {
        send_file("./files/index_script.js", "application/javascript");
    }

    // Route for serving the reference made by index.html to lion_sleeping.jpg --> once the index.html is sent to the browser the browser will make another GET request for each of the referenced files inside index.html.
    ROUTE_GET("/lion_sleeping.jpg")
    {
        send_file("./files/lion_sleeping.jpg", "image/jpeg");
    }

    // Route for handling user login
    ROUTE_POST("/login")
    {
        fprintf(stderr, "[DEBUG] Received Login Payload: %s\n", payload);

        if (login_user(payload)) 
        {
            char username[100] = {0};
            sscanf(payload, "username=%[^&]", username);

            // Send JSON response instead of redirecting
            printf("HTTP/1.1 200 OK\r\n");
            printf("Content-Type: application/json\r\n\r\n");
            printf("{\"success\": true, \"username\": \"%s\"}", username);
            fflush(stdout);
        } 
        else 
        {
            // Return JSON response for failed login instead of redirecting
            printf("HTTP/1.1 401 Unauthorized\r\n");
            printf("Content-Type: application/json\r\n\r\n");
            printf("{\"success\": false, \"message\": \"Invalid email or password.\"}");
            fflush(stdout);
        }
    }



/* --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- */


    // Route for serving the profile page
    ROUTE_GET("/profile")
    {
        send_file("./files/profile_page.html", "text/html");
    }

    // Route for serving the reference made by profile_page.html to lion_awake.jpg --> once the profile_page.html is sent to the browser the browser will make another GET request for each of the referenced files inside index.html.
    ROUTE_GET("/lion_awake.jpg")
    {
        send_file("./files/lion_awake.jpg", "image/jpeg");
    }

    // Route for serving profile data
    ROUTE_GET("/profileinfo")
    {
        char username[100] = {0};
        char filepath[BUF_SIZE] = {0};
        char profile_data[BUF_SIZE] = {0};

        sscanf(qs, "username=%s", username);

        snprintf(filepath, sizeof(filepath), "./data/%s.data", username);

        FILE *file = fopen(filepath, "r");
        if (!file) {
            printf("HTTP/1.1 404 Not Found\r\n\r\n");
            printf(" ");
            fflush(stdout);
            return;
        }

        fread(profile_data, 1, sizeof(profile_data), file);
        fclose(file);

        printf("HTTP/1.1 200 OK\r\n");
        printf("Content-Type: text/plain\r\n\r\n");
        printf("%s", profile_data);
        fflush(stdout);
    }


    ROUTE_GET("/profile_page.js")
    {
        send_file("./files/profile_page.js", "application/javascript");
    }






/*     ROUTE_POST("/")
    {
        printf("HTTP/1.1 200 OK\r\n\r\n"); // same as explained above in the previous routes.
        printf("Wow, seems that you POSTed %d bytes. \r\n", payload_size); // as explained above, and in addition it sends the payload size that was attached to the reqeust, which was declared as extern int in httpd.h and parsed and assign a value by the analayzed_http defined in the http_protocol.c file..
        printf("Fetch the data using `payload` variable."); // as explained above. 
    }
 */








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
/* ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- */
