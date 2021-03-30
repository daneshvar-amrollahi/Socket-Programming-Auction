#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdbool.h>

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0
#define NUM_PROJECTS 10

struct group
{
	int memberfd[5];
};
struct group waiting_list[10];


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
        error("ERROR on accept");
    return newsockfd;
}

int client_fd[50]; //client_fd[i]: i'th client fd
int client_count = 0; //number of clients
char buffer[255];
int project_volunteers[10];
int buf_idx; //index for buffer

void add_num_to_buffer(int num)
{
    
    if (num == 0)
    {
        buffer[19] = '0';
        buffer[20] = ',';
        buf_idx = 21;
        return;
    }
    char d[5];
    int idx = 0;
    while (num)
    {
        d[idx++] = (num % 10) + '0';
        num /= 10;
    }
    idx--;
    for (idx ; idx >= 0 ; idx--)
        buffer[buf_idx++] = d[idx]; 
    
    buffer[buf_idx++] = ',';
    
}

void write_projects_for_client(int client_fd)
{
    buf_idx = 19;
    bzero(buffer, 255);
    strcat(buffer, "Available Projects:");
    for (int i = 0 ; i < 10 ; i++)
    {
        if (project_volunteers[i] < 5) //project is available for offering
            add_num_to_buffer(i);
    }
    buffer[buf_idx - 1] = '\n';
    int n = write(client_fd, buffer, strlen(buffer));
    if (n < 0)
        error("ERROR writing to client socket");
}

void handle_group(int project_num)
{
	for (int i = 0 ; i < 5 ; i++)
	{
		int cur_fd = waiting_list[project_num].memberfd[i];
		
		bzero(buffer, 255);
		strcat(buffer, "The auction will begin now. Go go go!");
		write(cur_fd, buffer, strlen(buffer));
	}
    
}

int assign_project_to_client(int clientfd, char project_num)
{
	waiting_list[project_num - '0'].memberfd[project_volunteers[project_num - '0']] = clientfd;
    project_volunteers[project_num - '0']++;
    bzero(buffer, 255);

    strcat(buffer, "Please connect to port ");
    char port_num[5] = {'8', '0', '0', project_num, '\0'};
    strcat(buffer, port_num);

    strcat(buffer, ". You are person number ");
    char turn[2] = {project_volunteers[project_num - '0'] + '0', '\0'};
    strcat(buffer, turn);
    strcat(buffer, " in the queue for this project when offering prices.\n\0");

    
    int n = write(clientfd, buffer, strlen(buffer));
    if (n < 0)
        error("ERROR on wiring message to client from server\n");

	if (project_volunteers[project_num - '0'] == 5) //group is full
	{
		handle_group(project_num - '0');
	}
 }



int handle_connection(int clientfd)
{
    bzero(buffer,255);
    int n = read(clientfd, buffer, 255);
    if (n == 0) //EOF
        return 0;
    if (n < 0)
        error("ERROR reading from client\n");

    printf("MESSAGE FROM CLIENT: %d: %s\n\n", clientfd, buffer);
    if (buffer[0] == 'V') //volunteered
    {
        char project_num = buffer[2];
        assign_project_to_client(clientfd, project_num);
    }

    return 1;
}

int main(int argc, char const *argv[])
{
    int server_socket_fd, newsockfd, port;
    socklen_t clilen;
    
    
    if (argc < 2) 
    {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(EXIT_FAILURE);
    }
    port = atoi(argv[1]);
    
    server_socket_fd = socket(AF_INET, SOCK_STREAM, 0); //create server socket (TCP)
    if (server_socket_fd < 0) 
        error("ERROR opening server socket");
    
    struct sockaddr_in serv_addr; 
    bzero((char *) &serv_addr, sizeof(serv_addr));    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);


    if (bind(server_socket_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR on binding");
    listen(server_socket_fd,30);



    fd_set current_sockets, ready_sockets;
    FD_ZERO(&current_sockets);
    FD_SET(server_socket_fd, &current_sockets);

    while (1)
    {
        ready_sockets = current_sockets;

    	if (select(FD_SETSIZE, &ready_sockets, NULL, NULL, NULL) < 0)
            error("ERROR on select");

        for (int i = 0 ; i < FD_SETSIZE ; i++)
        {
            if (FD_ISSET(i, &ready_sockets)) //i is a fd with data that we can read
            {
                if (i == server_socket_fd)
                {
                    printf("There is a new connection request from a client...\n");

                    int client_socket = accept_new_connection(server_socket_fd);

                    client_fd[client_count++] = client_socket;
                    FD_SET(client_socket, &current_sockets);
                    printf("New connection accepted with fd %d\n", client_socket);
                    printf("Total No. of Clients til now %d\n\n", client_count); 
                    write_projects_for_client(client_socket);
                }
                else
                {

                    //1. Client has FD_CLR(i, &current_sockets);chosen a project
                    //2. Client has announced their result in their group
                    int n = handle_connection(i);
                    if (n == 0) //EOF event happened
                        FD_CLR(i, &current_sockets);
                    //FD_CLR(i, &current_sockets);
                }
            }
        }
    }
    return 0;
}