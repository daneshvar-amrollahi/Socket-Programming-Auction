#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

void error(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char const *argv[])
{
    int server_fd, port;
    if (argc < 2)
    {
        fprintf(stderr,"ERROR: No Port Provided\n");
        exit(EXIT_FAILURE);
    }

    port = atoi(argv[1]);
    
    return 0;
}