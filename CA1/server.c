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

int accept_new_connection(int server_socket)
{
    struct sockaddr_in cli_addr;
    socklen_t clilen;

    clilen = sizeof(cli_addr);
    int newsockfd = accept(server_socket, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) 
    {
        error("ERROR on accept");
        exit(EXIT_FAILURE);
    }
    return newsockfd;
}

int main(int argc, char const *argv[])
{
    int server_socket_fd, newsockfd, port;
    socklen_t clilen;
    char buffer[255];
    struct sockaddr_in serv_addr;//, cli_addr;
    int n;

    if (argc < 2) 
    {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(EXIT_FAILURE);
    }
    port = atoi(argv[1]);
    
    server_socket_fd = socket(AF_INET, SOCK_STREAM, 0); //create server socket (TCP)
    if (server_socket_fd < 0) 
        error("ERROR opening server socket");
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);


    if (bind(server_socket_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR on binding");
    listen(server_socket_fd,5);
    //clilen = sizeof(cli_addr);


    int client_fd[50];
    int client_count = 0;

    fd_set current_sockets, ready_sockets;

    FD_ZERO(&current_sockets);
    FD_SET(server_socket_fd, &current_sockets);

    while (1)
    {
        ready_sockets = current_sockets;

        if (select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL) < 0)
        {
            perror("ERROR: select()");
            exit(EXIT_FAILURE);
        }

        for (int i = 0 ; i < FD_SETSIZE ; i++)
        {
            if (FD_ISSET(i, &ready_sockets)) //i is a fd with data that we can read
            {
                if (i == server_socket_fd)
                {
                    printf("This is server.c\n There is a new connection to accept\n");

                    int client_socket = accept_new_connection(server_socket_fd);

                    client_fd[client_count++] = client_socket;
                    FD_SET(client_socket, &current_sockets);
                    printf("New connection accepted\n");
                }
                else
                {
                    printf("This is server.c\n There seems to be a message from a client\n");
                    //handle_connection(i);
                    FD_CLR(i, &current_sockets);
                }
            }
        }
    }
    return 0;
}