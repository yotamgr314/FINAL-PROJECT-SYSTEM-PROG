#include "httpd.h"



int main(int c, char** v)
{
    serve_forever("8005"); // starts the https server.
    return 0;
}

