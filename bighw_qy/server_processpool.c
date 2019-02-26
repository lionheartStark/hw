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
#include<sys/wait.h>
#include<time.h>
#include "get_time.h"
#include "init_socket.h"
#include "http_session.h"

#define MAX_CONNECTION 10
#define MAX_PROCESS 10

int main(int argc, char** argv){
    int listen_fd;
    int connect_fd;
    socklen_t clientlen;

    struct sockaddr_in server_addr;
    //struct sockaddr_in client_addr;
    bzero(&server_addr, sizeof(struct sockaddr_in));
    //bzero(&client_addr, sizeof(struct sockaddr_in));

    socklen_t addrlen = sizeof(struct sockaddr_in);//套接字长度类型

    //创建监听套接字描述符
    if(init_socket(&listen_fd, &server_addr) == -1)
    {
        perror("init_socket() error. in server_iteration.c");
        exit(EXIT_FAILURE);
    }

    time_t ticks;
    pid_t pid; 
    pid_t pids[MAX_PROCESS];//后面将会一次产生十个子进程，存放每个子进程的id 

    //创建子进程
    int i;
    for(i = 0; i< MAX_PROCESS;i++){

        pid = fork();
        if(pid > 0){   
            pids[i] = pid;//continue;
            printf("new pid %d is %d\n",i,pid);                                                                                    continue;//父进程继续循环创建子进程
        }
        if(pid ==0){
            //子进程处理客户端的请求
            struct sockaddr_in client_addr;
            socklen_t addrlen = sizeof(struct sockaddr_in);//套接字长度类型

            while (1){
                memset(&client_addr, 0, sizeof (client_addr));
            
                if((connect_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addrlen)) == -1)
                {
                    perror("accept() error. in server_iteration.c");
                    continue;
                }
                //close(listen_fd);
                printf("%d get http session from %s:%d,%d\n", getpid(),inet_ntoa(client_addr.sin_addr), htons(client_addr.sin_port),pid);

                if(http_session(&connect_fd, &client_addr) == -1)
                {
                    perror("http_session() error. in server_iteration.c");
                    shutdown(connect_fd, SHUT_RDWR);
                    printf("loss connection to %s\n", inet_ntoa(client_addr.sin_addr));
                    exit(EXIT_FAILURE);
                }
                printf("%d close connection to %s\n", getpid(),inet_ntoa(client_addr.sin_addr));
                shutdown(connect_fd, SHUT_RDWR);
            }
        }
        
        
    }
    close(listen_fd);
    printf("main process closed listen\n");
    //等待回收子进程
    for(i = 0; i< MAX_PROCESS;i++){
        int status;
        if(pids[i] < 0){
            continue;
        }
        printf("wait process %d\n",pids[i]);
        waitpid(pids[i],&status,0);
    }
    return 0;
}
