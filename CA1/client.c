#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <signal.h>
#include <time.h>
#include <errno.h>

#define STDIN 0

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

#define USERS_FOR_PROJECT 3

int turn_cnt = 0;
volatile sig_atomic_t time_is_up = 0;
void sig_handler(int signum)
{ 
    time_is_up = 1;
    //turn_cnt++;
    //if (turn_cnt != 0)
    //    printf("Time is up!\n");
    //printf("Person number %d in queue has 10 seconds to offer his/her price...\n", ++turn_cnt);
    //alarm(10);
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
        error("ERROR connecting");


    printf("I am connected to the server now\n");

    fd_set current_sockets;

    int udp_port, udp_sockfd;



    while (1)
    {
        FD_ZERO(&current_sockets);
        FD_SET(sockfd, &current_sockets); //socket for connection with server
        FD_SET(STDIN, &current_sockets);  //standard input 

        if (select(sockfd + 1, &current_sockets, NULL, NULL, NULL) < 0)
            perror("ERROR on select");
        

        
        if (FD_ISSET(sockfd, &current_sockets)) 
        {
            if (sockfd == sockfd) //server has written something for this client
            {
                bzero(buffer, 255);
                int n = read(sockfd, buffer, 255); //read message from server
                if (n < 0)
                    error("ERROR for client in reading buffer from server\n");
                

                printf("MESSAGE FROM SERVER: %s\n\n", buffer);

                if (buffer[0] == 'P') //Please connect to port ....
                {
                    printf("Connecting to UDP Socket...\n");
                    char port_numm[4] = {'8', '0', '0', buffer[26]};
                    udp_port = atoi(port_numm);
                }

                if (buffer[0] =='T') //The auction will begin now!
                {
                    FD_CLR(sockfd, &current_sockets);
                    FD_CLR(STDIN, &current_sockets);
                    //close(sockfd);
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
                error("ERROR writing volunteered project\n");
        }

    }

    udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sockfd < 0)
        error("ERROR opening socket\n");
    
    int broadcast_en = 1, opt_en = 1, reuse_adr_en = 1;
    setsockopt(udp_sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast_en, sizeof(broadcast_en)); 
    setsockopt(udp_sockfd, SOL_SOCKET, SO_REUSEPORT, &opt_en, sizeof(opt_en));
    setsockopt(udp_sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse_adr_en, sizeof(reuse_adr_en));
    
    struct sockaddr_in bc_adr_sendto, bc_adr_recvfrom;

    bc_adr_recvfrom.sin_family = AF_INET;
    bc_adr_recvfrom.sin_addr.s_addr = htonl(INADDR_ANY); //INADDR_ANY 
    bc_adr_recvfrom.sin_port = htons(udp_port);

    bc_adr_sendto.sin_family = AF_INET;
    bc_adr_sendto.sin_addr.s_addr = htonl(INADDR_BROADCAST); 
    bc_adr_sendto.sin_port = htons(udp_port);

    
    if (bind(udp_sockfd, (struct sockaddr *) &bc_adr_recvfrom, sizeof(bc_adr_recvfrom)) < 0) 
        error("ERROR on binding");
    

    printf("I am connected to the UDP Socket now\n\n");
    //Currently connected to the UDP socket


    signal(SIGALRM, sig_handler); //Register signal handler
    printf("Auction beginning...\n");
    alarm(10);
    int mn = 2e9, ans;
    int my_offered_price = 0;
    while (1)
    {
        signal(SIGALRM, sig_handler); //Register signal handler
        FD_ZERO(&current_sockets);
        FD_SET(udp_sockfd, &current_sockets);
        FD_SET(STDIN, &current_sockets);  //standard input 

        int res = select(udp_sockfd + 1, &current_sockets, NULL, NULL, NULL);

        if (res == -1)
        {
            if (errno == EINTR)
            {
                printf("Time is up\n");
                time_is_up = 0;
                turn_cnt++;
                alarm(10);
                continue;
            }
            else
            {   
                error("ERROR select\n");
            }
        }   

        
        if (FD_ISSET(STDIN, &current_sockets)) 
        {
            bzero(buffer, 255);
            fgets(buffer, 255, stdin);

            
            int price = atoi(buffer);

            my_offered_price = price;

            if (price < mn)
            {
                mn = price;
                ans = turn_cnt;
            }

            bzero(buffer, 255);
            strcat(buffer, "Person with turn ");
            char c[2] = {turn_cnt + '0', '\0'};
            strcat(buffer, c);
            strcat(buffer, " offered price ");
            
            char d[10]; //digits of price
            int sz = 0;
            while (price)
            {
                d[sz++] = '0' + (price % 10);
                price /= 10; 
            }         
            char rd[10];
            for (int i = sz - 1 ; i >= 0 ; i--)
                rd[sz - 1 - i] = d[i];
            rd[sz] = '\n';
            rd[sz + 1] = '\0';
        
            strcat(buffer, rd);
            
            

            sendto(udp_sockfd, buffer, strlen(buffer), 0,(struct sockaddr *)&bc_adr_sendto, sizeof(struct sockaddr_in));   

            
            
        }

        if (FD_ISSET(udp_sockfd, &current_sockets)) //something broadcasted on udp_sockfd
        {
            turn_cnt++;
            bzero(buffer, 255);
            socklen_t bc_adr_len = sizeof(bc_adr_recvfrom);
            int n = recvfrom(udp_sockfd, buffer, 255, 0, (struct sockaddr *)&bc_adr_recvfrom, &bc_adr_len);
            if (n < 0)
                error("ERROR on reading broadcasted message\n");
            printf("BROADCASTED MESSAGE: %s\n", buffer);


            int offered_price = 0;
            //printf("salammm\n");
            int idx = 254;
            while (1)
            {
                if (buffer[idx] == 'e')
                    break;
                idx--;
            }

            for (int j = idx + 2 ; buffer[j] >= '0' && buffer[j] <= '9' ; j++)
                offered_price = offered_price * 10 + (buffer[j] - '0');

            //printf("offered price is %d\n", offered_price);

            int offered_id;
            idx = 254;
            while (1)
            {
                if (buffer[idx] == 'n')
                    break;
                idx--;
            }
            offered_id = buffer[idx + 2] - '0';

            //printf("offered id is %d\n", offered_id);

            if (offered_price < mn)
            {
                mn = offered_price;
                ans = offered_id;
            }

            fflush(stdout);
        }
        
        alarm(10);

        if (turn_cnt == USERS_FOR_PROJECT)
            break;
    }
    
    printf("The minimum price offer is %d\n", mn);

    if (mn == my_offered_price) //I am the winner
    {
        bzero(buffer, 255);
        strcat(buffer, "I am the winner! My offered price is ");

        int price = mn;
        char d[10]; //digits of price
        int sz = 0;
        while (price)
        {
            d[sz++] = '0' + (price % 10);
            price /= 10; 
        }         
        char rd[10];
        for (int i = sz - 1 ; i >= 0 ; i--)
            rd[sz - 1 - i] = d[i];
        rd[sz] = '\n';
        rd[sz + 1] = '\0';
    
        strcat(buffer, rd);
        
        write(sockfd, buffer, strlen(buffer));
    }
    return 0;
}
