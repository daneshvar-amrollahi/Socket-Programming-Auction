#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define STDIN 0

void error(const char *msg)
{
    perror(msg);
    exit(0);
}



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

    fd_set current_sockets;

    int udp_port, udp_sockfd;


    while (1)
    {
        FD_ZERO(&current_sockets);
        FD_SET(sockfd, &current_sockets); //socket for connection with server
        FD_SET(STDIN, &current_sockets);  //standard input 

        if (select(sockfd + 1, &current_sockets, NULL, NULL, NULL) < 0)
        {
            perror("ERROR on select");
            exit(EXIT_FAILURE);
        }

        
        if (FD_ISSET(sockfd, &current_sockets)) 
        {
            if (sockfd == sockfd) //server has written something for this client
            {
                bzero(buffer, 255);
                int n = read(sockfd, buffer, 255); //read message from server
                if (n < 0)
                {
                    error("ERROR for client in reading buffer from server\n");
                    exit(EXIT_FAILURE);
                }

                printf("MESSAGE FROM SERVER: %s\n", buffer);

                if (buffer[0] == 'P') //Please connect to port ....
                {
                    printf("Connecting to UDP Socket...\n");
                    char port_numm[4] = {'8', '0', '0', buffer[26]};
                    udp_port = atoi(port_numm);

                    udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
                    if (udp_sockfd < 0)
                    {
                        error("ERROR opening socket\n");
                        exit(EXIT_FAILURE);
                    }
                    if (setsockopt(udp_sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
                    {
                        error("setsockopt(SO_REUSEADDR) failed");
                        exit(EXIT_FAILURE);
                    }

                    struct sockaddr_in bc; //bc: broadcasting
                    bc.sin_family = AF_INET;
                    bc.sin_addr.s_addr = INADDR_ANY;
                    bc.sin_port = htons(udp_port);

                    if (bind(udp_sockfd, (struct sockaddr *) &bc, sizeof(bc)) < 0) 
                        error("ERROR on binding");


                    printf("I am connected to the UDP Socket now\n");
                }

                if (buffer[0] =='B') //Be ready!
                {
                    FD_CLR(sockfd, &current_sockets);
                    FD_CLR(STDIN, &current_sockets);
                    close(sockfd);
                    break;
                }
            }
        }

        if (FD_ISSET(STDIN, &current_sockets))
        {
            bzero(buffer, 255);
            fgets(buffer, 255, stdin);
            //V X: Volunteered for project X
            int n = write(sockfd, buffer, strlen(buffer));
            if (n < 0)
            {
                error("ERROR writing volunteered project\n");
                exit(EXIT_FAILURE);
            }
        }

    }


    //Currently connected to the UDP socket
    printf("outside while\n");

    
    return 0;
}