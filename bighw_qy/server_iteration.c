#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "get_time.h"
#include "init_socket.h"
#include "http_session.h"

int main(int argc, char *argv[])
{
    int listen_fd;
    int connect_fd;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    bzero(&server_addr, sizeof(struct sockaddr_in));
    bzero(&client_addr, sizeof(struct sockaddr_in));
    
    if(init_socket(&listen_fd, &server_addr) == -1)
    {
        perror("init_socket() error. in server_iteration.c");
        exit(EXIT_FAILURE);
    }
    
    socklen_t addrlen = sizeof(struct sockaddr_in);
    pid_t pid;
    
    while(1)
    {
        if((connect_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addrlen)) == -1)
        {
            perror("accept() error. in server_iteration.c");
            continue;
        }
        
        printf("http session from %s:%d\n", inet_ntoa(client_addr.sin_addr), htons(client_addr.sin_port));
        if(http_session(&connect_fd, &client_addr) == -1)
        {
            perror("http_session() error. in server_iteration.c");
            shutdown(connect_fd, SHUT_RDWR);
            printf("loss connection to %s\n", inet_ntoa(client_addr.sin_addr));
            exit(EXIT_FAILURE);
        }
        
        printf("we close connection to %s:%d\n", inet_ntoa(client_addr.sin_addr),htons(client_addr.sin_port));
        shutdown(connect_fd, SHUT_RDWR);
    }
    
    shutdown(listen_fd, SHUT_RDWR);
    return 0;
}