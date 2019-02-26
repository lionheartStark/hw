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
#include "threadpool.h"



#define MAXLINE 1024

struct argument
{
  int connect_fd;
  struct sockaddr_in client_addr;
};



//回声线程
void *echo_thread(void *argv)
{
    
    struct argument * thread_argv =( struct argument *) argv;
    int connect_fd = thread_argv->connect_fd;
    struct sockaddr_in client_addr = thread_argv->client_addr;

    
    //获得进程id并分离线程
    pthread_detach(pthread_self());

    //http服务
    if(http_session(&connect_fd, &client_addr) == -1)
    {
        perror("http_session() error. in server_iteration.c");
        shutdown(connect_fd, SHUT_RDWR);
        printf("loss connection to %s\n", inet_ntoa(client_addr.sin_addr));
        exit(EXIT_FAILURE);
    }
    
    printf("close connection to %s\n", inet_ntoa(client_addr.sin_addr));
    shutdown(connect_fd, SHUT_RDWR);
    free((struct sockaddr_in*)argv);

    return NULL;
}

int main(int argc, char *argv[])
{
    int listen_fd;
    int connect_fd;
    socklen_t clientlen;
    /*              
    struct sockaddr_storage clientaddr; //适应于任何地址
    char client_hostname[MAXLINE];      //主机名
    char client_port[MAXLINE];          //端口
    pthread_t tid;                      //线程id
    */

    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    bzero(&server_addr, sizeof(struct sockaddr_in));
    bzero(&client_addr, sizeof(struct sockaddr_in));
    
    socklen_t addrlen = sizeof(struct sockaddr_in);//套接字长度类型

    //创建监听套接字描述符
    if(init_socket(&listen_fd, &server_addr) == -1)
    {
        perror("init_socket() error. in server_iteration.c");
        exit(EXIT_FAILURE);
    }
    
    //创建线程池
    struct threadpool *pool = threadpool_init(10, 20);

    while(1)
    {
        if((connect_fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addrlen)) == -1)
        {
            perror("accept() error. in server_iteration.c");
            continue;
        }
        

        printf("http session from %s:%d\n", inet_ntoa(client_addr.sin_addr), htons(client_addr.sin_port));

        //添加线程任务
        struct argument * thread_argv;
        thread_argv = malloc(sizeof(struct argument));
        thread_argv->connect_fd=connect_fd;
        memcpy((void*)&(thread_argv->client_addr),&client_addr,sizeof(client_addr));
        threadpool_add_job(pool, echo_thread, (void*)thread_argv);
        /*
        if(http_session(&connect_fd, &client_addr) == -1)
        {
            perror("http_session() error. in server_iteration.c");
            shutdown(connect_fd, SHUT_RDWR);
            printf("loss connection to %s\n", inet_ntoa(client_addr.sin_addr));
            exit(EXIT_FAILURE);
        }
        
        printf("close connection to %s\n", inet_ntoa(client_addr.sin_addr));
        shutdown(connect_fd, SHUT_RDWR);
        */
    }
    
    shutdown(listen_fd, SHUT_RDWR);
    return 0;
}