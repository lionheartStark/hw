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
#include <pthread.h>
#include "get_time.h"
#include "init_socket.h"
#include "http_session.h"
#include "sembuf.h"
#define NTHREAD 10
#define SBSIZE  16

//基于信号量的缓冲区
sembuf_t sb;
struct argument
{
  int connect_fd;
  struct sockaddr_in client_addr;
};
//线程工作
void *echo(void* argv)
{
      
    pthread_detach(pthread_self());
    while (1) {
        int connect_fd = sembuf_remove(&sb); //获得一个可用消费项
        //回声服务
        if(http_session(&connect_fd, NULL) == -1)
        {
            perror("http_session() error. in server_iteration.c");
            shutdown(connect_fd, SHUT_RDWR);
            printf("loss connection");
            //printf("loss connection to %s\n", inet_ntoa(client_addr.sin_addr));
            exit(EXIT_FAILURE);   
        }
        printf("close connection\n");
        //关闭连接
        shutdown(connect_fd, SHUT_RDWR);
        
    }

    return NULL;
}



int main(int argc, char *argv[])
{
    int listen_fd;
    int connect_fd;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    bzero(&server_addr, sizeof(struct sockaddr_in));
    bzero(&client_addr, sizeof(struct sockaddr_in));
    pthread_t tid;                      //线程id

    if(init_socket(&listen_fd, &server_addr) == -1)
    {
        perror("init_socket() error. in server_iteration.c");
        exit(EXIT_FAILURE);
    }
    
    socklen_t addrlen = sizeof(struct sockaddr_in);
    pid_t pid;
    //初始化缓冲区
    sembuf_init(&sb, SBSIZE);
     //创建线程
    for(int i = 0; i < NTHREAD; i++){
        pthread_create(&tid, NULL, echo, NULL);
    }
    while(1)
    {
        if((connect_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addrlen)) == -1)
        {
            perror("accept() error. in server_iteration.c");
            continue;
        }
        
        printf("http session from %s:%d\n", inet_ntoa(client_addr.sin_addr), htons(client_addr.sin_port));
        
         //添加消费项
         //添加线程任务
         /*
        struct argument * thread_argv;
        thread_argv = malloc(sizeof(struct argument));
        thread_argv->connect_fd=connect_fd;
        memcpy((void*)&(thread_argv->client_addr),&client_addr,sizeof(client_addr));
        */
        sembuf_insert(&sb, connect_fd);
        
    }
    
    shutdown(listen_fd, SHUT_RDWR);
    return 0;
}