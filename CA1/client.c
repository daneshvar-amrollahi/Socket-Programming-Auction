#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

#define STDIN 0

int main(int argc, char *argv[])
{
    int sockfd, port;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    if (argc < 3) 
    {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(EXIT_FAILURE);
    }
    port = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(EXIT_FAILURE);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(port);
    
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
    {
        error("ERROR connecting");
        exit(EXIT_FAILURE);
    }

    printf("I am connected to the server now\n");

    fd_set current_sockets, ready_sockets;
    FD_ZERO(&current_sockets);
    FD_SET(sockfd, &current_sockets); //socket for connection with server
    FD_SET(STDIN, &current_sockets);  //standard input 

    printf("sockfd is %d\n", sockfd);

    while (1)
    {
        ready_sockets = current_sockets;

        if (select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL) < 0)
        {
            perror("ERROR on select");
            exit(EXIT_FAILURE);
        }

        for (int i = 0 ; i < FD_SETSIZE ; i++)
        {
            if (FD_ISSET(i, &ready_sockets)) //i is a fd with data that we can read
            {
                printf("event here on %d\n", i);
                if (i == sockfd) //server has written something for this client
                {
                    bzero(buffer, 255);
                    int n = read(sockfd, buffer, 255); //read message from server: 1. projects list 2. new port and turn
                    if (n < 0)
                    {
                        error("ERROR for client in reading buffer from server\n");
                        exit(EXIT_FAILURE);
                    }

                    printf("Message from Server: %s\n", buffer);
                }
            }
            if (FD_ISSET(STDIN, &ready_sockets))
            {
                bzero(buffer, 255);
                fgets(buffer, 255, stdin);
                printf("Input from user is %s\n", buffer);
                //V X: Volunteered for project X
                int n = write(sockfd, buffer, strlen(buffer));
                if (n < 0)
                {
                    error("ERROR writing volunteered project\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
        FD_SET(sockfd, &current_sockets); //socket for connection with server
        FD_SET(STDIN, &current_sockets);  //standard input 
    }

    close(sockfd);
    return 0;
}