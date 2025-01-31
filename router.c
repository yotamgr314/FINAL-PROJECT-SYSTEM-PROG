// NOTE: DONE COPING COMMENTS.

// NOTE: THIS FILE HANDLES ROUTING OF HTTP REQUESTS.
//       01) Defines routes for GET and POST requests.
//       02) Calls appropriate functions to handle requests.
//       03) Implements business logic for user registration, login, and profile management.
// NOTE: each printf statment written here is actully being sent over directly to the clientSocketDescritor becuse right before we called the void route() function we used dup2 to redirected the child process STDOUT to the clientSocketDescritor. 
//       hence there is no need to use send() API function here. 
 

#include "httpd.h"
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <ctype.h>

#define BUF_SIZE 1024

// CONTROLERS SECTION
/* ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- */
// Function to decode URL-encoded characters (e.g., `%40` → `@`), The email entered yotamkiki@gmail.com appears as yotamkiki%40gmail.com. the  @ symbol is URL-encoded as %40 (because form data is sent using application/x-www-form-urlencoded

// Function to trim whitespace and newlines from a string
void trim_newline(char *str) {
    int len = strlen(str);
    while (len > 0 && (str[len - 1] == '\n' || str[len - 1] == '\r' || isspace(str[len - 1]))) {
        str[len - 1] = '\0';
        len--;
    }
}

void url_decode(const char *src, char *dest) {
    char code[3] = {0};
    while (*src) {
        if (*src == '%' && isxdigit(src[1]) && isxdigit(src[2])) {
            code[0] = src[1];
            code[1] = src[2];
            *dest++ = (char)strtol(code, NULL, 16);  // Convert `%xx` to char
            src += 3;
        } else if (*src == '+') {
            *dest++ = ' ';  // Convert `+` to space
            src++;
        } else {
            *dest++ = *src++;
        }
    }
    *dest = '\0';  // Null-terminate decoded string
}


// find function - returns 1 if username exists in in password.txt, and 0 if not.
int user_exists(const char* username) {
    FILE *file = fopen("./files/password.txt", "r");
    if (!file) {
        fprintf(stderr, "[ERROR] Could not open password.txt\n");
        return 0;
    }

    char stored_payload[BUF_SIZE];  // Line buffer
    char stored_username[100];      // Extracted username
    char decoded_stored_username[100];  // Decoded stored username
    char decoded_input_username[100];   // Decoded input username

    // Decode input username
    url_decode(username, decoded_input_username);
    fprintf(stderr, "[DEBUG] Checking if user exists: %s\n", decoded_input_username);

    while (fgets(stored_payload, sizeof(stored_payload), file)) {// scans into stored_payload all the usernames from passwordFileDescritor and if a username matches the newUserName it returns 1, else 0.
        trim_newline(stored_payload);  // Remove newlines

        // Extract username from stored data
        if (sscanf(stored_payload, "username=%99[^&]", stored_username) == 1) {
            trim_newline(stored_username);  // Trim whitespace

            // Decode stored username
            url_decode(stored_username, decoded_stored_username);

            fprintf(stderr, "[DEBUG] Comparing input: %s with stored: %s\n",
                    decoded_input_username, decoded_stored_username);

            if (strcmp(decoded_input_username, decoded_stored_username) == 0) {
                fclose(file);
                return 1;  // Username exists
            }
        }
    }

    fclose(file);
    return 0;  // Username does not exist
}





// writes into the password.txt the new user name and password in application/x-www-form-urlencoded format - username=yotamgr@gmail.com&password=123. returns 0 if username do not exist in the payload(body) of the request. 1 if succeed to write into password.txt.
int register_user(const char* payload) 
{
    if (strstr(payload, "username=") == NULL) // strstr searches fro the substring username inside the payloard string.
    {
        return 0;
    }

    char decoded_payload[1024];  // Buffer for decoded data
    url_decode((char *)payload, decoded_payload);  // Decode URL-encoded data

    FILE *file = fopen("./files/password.txt", "a"); // the "a" mode means append mode, adds to the end of the file, and if the file do not exist it opens it. 
    if (!file) {
        return 0;
    }

    fprintf(file, "%s\n", decoded_payload);  // prints the payload into the password.txt file, via changing the fprintf stdout into the passwordFileDescriptor.
    fclose(file);
    return 1;
}

// Function to check if a user exists in password.txt, if he does return 1, else return 0.
int login_user(const char* payload)
{
    FILE *file = fopen("./files/password.txt", "r");
    if (!file) {
        return 0;
    }

    char stored_payload[BUF_SIZE];  
    char decoded_payload[BUF_SIZE];

    // Decode only the input payload (user's inserted login email switch %40 to @)
    url_decode((char *)payload, decoded_payload);
    fprintf(stderr, "[DEBUG] Decoded login payload: %s\n", decoded_payload);

    // Read each line from the password file
    while (fgets(stored_payload, sizeof(stored_payload), file)) 
    {
        stored_payload[strcspn(stored_payload, "\n")] = '\0';

        // Compare decoded input with stored credentials
        if (strcmp(decoded_payload, stored_payload) == 0) 
        {
            fclose(file);
            return 1;  // Login successful will return 1 if the credentials match.
        }
    }

    fclose(file);
    return 0;  // return 0 if no match is found -  Login failed
}


// Generic Cenetrelized logic for send files method which utilizied printf due the dup2. 
void send_file(const char *file_path, const char *content_type)
{
    int fd = open(file_path, O_RDONLY); // gets the index.html/css/imagefile file descritor. 
    if (fd == -1) {
        // Send proper 404 response
        printf("HTTP/1.1 404 Not Found\r\n");
        printf("Content-Type: text/plain\r\n");
        printf("Connection: close\r\n");
        printf("\r\n");  // Ensure correct separation of headers and body
        printf("Error: Could not open %s.\r\n", file_path);
        fflush(stdout);
        return;
    }

    char buffer[BUF_SIZE];
    ssize_t bytes_read;

    // Send headers
    printf("HTTP/1.1 200 OK\r\n");
    printf("Content-Type: %s\r\n", content_type);
    printf("Connection: close\r\n");
    printf("\r\n");  // Ensure proper separation before content
    fflush(stdout);

    // Send file content
    while ((bytes_read = read(fd, buffer, BUF_SIZE)) > 0) {
        fwrite(buffer, 1, bytes_read, stdout); 
        fflush(stdout);
    }

    close(fd);
}

/* ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- */

// ROUTES SECTION
/* ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- */

void route() {
    ROUTE_START()
/* --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- */
    // NOTE: THESE ARE THE ROTUES FOR INDEX.HTML PAGE.

    // Route for serving the main HTML page
    ROUTE_GET("/") // MACRO for GET http request, defined in httpd.h
    {
        // write to the STDO which is redirected to the clientSocketDescritor a standard HTTP 200 OK.
        // write to the STDO which is redirected to the clientSocketDescritor the request header parsed in http_protocol.c, once we return the from void route() to the httpd.c we make call fflush(stdout)Make sure any buffered output is sent to the clientSocketDescritor.  
        // load the lion image.

        send_file("./files/index.html", "text/html");
    }

    // Route for serving the reference made by index.html to index_style.css --> once the index.html is sent to the browser the browser will make another GET request for each of the referenced files inside index.html.
    ROUTE_GET("/index_style.css")
    {
        send_file("./files/index_style.css", "text/css");
    }

    ROUTE_GET("/profile_page.css")
    {
        send_file("./files/profile_page.css", "text/css");
    }

    // Route for serving the reference made by index.html to index_script.js --> once the index.html is sent to the browser the browser will make another GET request for each of the referenced files inside index.html.
    ROUTE_GET("/index_script.js")
    {
        send_file("./files/index_script.js", "application/javascript");
    }

    ROUTE_GET("/profile_page.js")
    {
        send_file("./files/profile_page.js", "application/javascript");
    }

    // Route for serving the reference made by index.html to lion_sleeping.jpg --> once the index.html is sent to the browser the browser will make another GET request for each of the referenced files inside index.html.
    ROUTE_GET("/lion_sleeping.jpg")
    {
        send_file("./files/lion_sleeping.jpg", "image/jpeg");
    }

    // Route for serving the reference made by profile_page.html to lion_awake.jpg --> once the profile_page.html is sent to the browser the browser will make another GET request for each of the referenced files inside index.html.
    ROUTE_GET("/lion_awake.jpg")
    {
        send_file("./files/lion_awake.jpg", "image/jpeg");
    }

    // Route for serving the profile page
    ROUTE_GET("/profile")
    {
        send_file("./files/profile_page.html", "text/html");
    }

    // Route for serving profile data gets the data in data/yotamgr@gmail.com.data file.
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

    // Route for handling user registration
ROUTE_POST("/register") 
{
    char username[100] = {0};
    sscanf(payload, "username=%99[^&]", username);
    trim_newline(username); // Ensure clean input

    if (user_exists(username)) {
        printf("HTTP/1.1 302 Found\r\n"); 
        printf("Location: /?response=RegisterFailed\r\n\r\n"); 
        fflush(stdout);
    } else {
        if (register_user(payload)) { 
            printf("HTTP/1.1 302 Found\r\n"); // write to the STDO which is redirected to the clientSocketDescritor a standard HTTP 302 Found, as the route was found. - once we return the from void route() to the httpd.c we make call fflush(stdout)Make sure any buffered output is sent to the clientSocketDescritor.
            printf("Location: /?response=RegisterSuccess\r\n\r\n");  // as explained above but write to it a redirect path.
            fflush(stdout);
        } else {
            printf("HTTP/1.1 302 Found\r\n");
            printf("Location: /?response=RegisterFailed\r\n\r\n");
            fflush(stdout);
        }
    }
}

    // Route for handling user login
    ROUTE_POST("/login")
    {
        if (login_user(payload)) {
            char username[100] = {0};
            sscanf(payload, "username=%[^&]", username);
            printf("HTTP/1.1 302 Found\r\n");
            printf("Location: /profile?username=%s\r\n\r\n", username);
            fflush(stdout);
        } else {
            printf("HTTP/1.1 302 Found\r\n");
            printf("Location: /?response=LoginFailed\r\n\r\n");
            fflush(stdout);
        }
    }
    // Route for updating profile information
    ROUTE_POST("/profileinfo")
    {
        char username[100] = {0};
        char profileText[BUF_SIZE] = {0};
        char filepath[BUF_SIZE] = {0};

        sscanf(qs, "username=%s", username);

        strncpy(profileText, payload, sizeof(profileText) - 1);

        snprintf(filepath, sizeof(filepath), "./data/%s.data", username);

        FILE *file = fopen(filepath, "w");
        if (!file) {
            printf("HTTP/1.1 500 Internal Server Error\r\n\r\n");
            printf("Error: Could not open profile file for user '%s'.", username);
            return;
        }

        fprintf(file, "%s", profileText);
        fclose(file);

        printf("HTTP/1.1 200 OK\r\n\r\n");
        printf("Profile updated successfully for user '%s'.", username);
        fflush(stdout);
    }

    // Add route for logout
    ROUTE_GET("/logout")
    {
        // Assuming no session management is needed, we just redirect to the index page.
        printf("HTTP/1.1 302 Found\r\n");
        printf("Location: /\r\n\r\n");
        fflush(stdout);
    }

    ROUTE_END()
}
